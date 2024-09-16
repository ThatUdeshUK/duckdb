#include "duckdb/execution/operator/projection/physical_predict.hpp"

#include <iostream>
#include <map>

#if defined (ENABLE_PREDICT) && defined (USE_TORCH)
#include "duckdb_torch.hpp"
#elif ENABLE_PREDICT
#include "duckdb_onnx.hpp"
#endif

#define CHUNK_PRED 1
// #define VEC_PRED 0

namespace duckdb {
class PredictState : public OperatorState {
public:
    explicit PredictState(ExecutionContext &context, unique_ptr<Predictor> p, PredictStats stats) : predictor(std::move(p)) {
        this->stats = stats;
    }

    unique_ptr<Predictor> predictor;
    PredictStats stats;

public:
    void Finalize(const PhysicalOperator &op, ExecutionContext &context) override {
        // std::cout << "Finanlize: " << std::endl;
        std::cout << "Load @run: " << stats.load << std::endl;
        std::cout << "Move @run: " << stats.move << std::endl;
        std::cout << "Predict @run: " << stats.predict << std::endl;
        std::cout << "Move Rev @run: " << stats.move_rev << std::endl;

        std::map<std::string, long> stats_map{{"load", stats.load}, {"move", stats.move}, {"predict", stats.predict}, {"move_rev", stats.move_rev}, {"correct", stats.correct}, {"total", stats.total}};

        context.thread.profiler.Flush(op, stats_map);
    }
};

PhysicalPredict::PhysicalPredict(vector<LogicalType> types_p, unique_ptr<PhysicalOperator> child)
    : PhysicalOperator(PhysicalOperatorType::PREDICT, std::move(types_p), child->estimated_cardinality) {
	children.push_back(std::move(child));
}

unique_ptr<Predictor> PhysicalPredict::InitPredictor() const {
#if defined(ENABLE_PREDICT) && USE_TORCH
    return make_uniq<TorchPredictor>();
#elif defined(ENABLE_PREDICT)
    return make_uniq<ONNXPredictor>();
#else
    return nullptr;
#endif
}

unique_ptr<OperatorState> PhysicalPredict::GetOperatorState(ExecutionContext &context) const {
	auto &client_config = ClientConfig::GetConfig(context.client);

    auto stats = make_uniq<PredictStats>();
    auto p = InitPredictor();
    p->task = static_cast<PredictorTask>(model_type);
    p->Config(client_config);
    p->Load(model_name, *stats);
    return make_uniq<PredictState>(context, std::move(p), *stats);
}

OperatorResultType PhysicalPredict::Execute(ExecutionContext &context, DataChunk &input, DataChunk &chunk,
                                          GlobalOperatorState &gstate, OperatorState &state_p) const {
    auto &state = state_p.Cast<PredictState>();

    DataChunk predictions;
    chunk.Split(predictions, input.ColumnCount());
    input.Copy(chunk);

    predictions.SetCardinality(input);

    auto &predictor = *state.predictor.get();
#if CHUNK_PRED
    if (predictor.task == PREDICT_TABULAR_TASK) {
        predictor.PredictChunk(input, predictions, (int) input.size(), this->input_mask, (int) result_set_types.size(), state.stats);
    } else if (predictor.task == PREDICT_LLM_TASK) {
        predictor.PredictLMChunk(input, predictions, (int) input.size(), this->input_mask, (int) result_set_types.size(), state.stats);
    }
#elif VEC_PRED
    std::vector<float> inputs;
    std::vector<float> outputs;
    for (idx_t row_idx = 0; row_idx < input.size(); row_idx++) {
        for (idx_t col_idx = 0; col_idx < input.ColumnCount(); col_idx++) {
            inputs.push_back(FloatValue::Get(input.GetValue(col_idx, row_idx)));
        }
    }
    predictor.PredictVector(inputs, outputs, (int) input.size(), (int) input.ColumnCount(), (int) result_set_types.size());
    int col_count = (int) result_set_types.size();
    idx_t idx = 0;
    for(const float& i : outputs) {
        predictions.SetValue(idx % col_count, idx / col_count, Value(i));
        idx++;
    }
    inputs.clear();
    outputs.clear();
#else
    if (predictor.task == PREDICT_TABULAR_TASK) {
        int output_size = (int) result_set_types.size();
        std::vector<float> inputs;
        std::vector<float> outputs;

        for (idx_t row_idx = 0; row_idx < input.size(); row_idx++) {
            for (idx_t col_idx = 0; col_idx < input.ColumnCount(); col_idx++) {
                inputs.push_back(FloatValue::Get(input.GetValue(col_idx, row_idx)));
            }
            predictor.Predict(inputs, outputs, output_size);
            idx_t res_idx = 0;
            for(const float& i : outputs) {
                predictions.SetValue(res_idx, row_idx, Value(i));
                res_idx++;
            }

            inputs.clear();
            outputs.clear();
        }
    } else if (predictor.task == PREDICT_LLM_TASK) {
        int output_size = (int) result_set_types.size();
        std::string input_str;
        std::vector<float> outputs;

        for (idx_t row_idx = 0; row_idx < input.size(); row_idx++) {
            input_str = StringValue::Get(input.GetValue(0, row_idx));

            predictor.PredictLM(input_str, outputs, output_size);
            idx_t res_idx = 0;
            for(const float& i : outputs) {
                predictions.SetValue(res_idx, row_idx, Value(i));
                res_idx++;
            }

            outputs.clear();
        }
    }
#endif

    int positives = 0;
    auto lhs = (float *) input.data[1].GetData();
    // auto rhs = (float *) predictions.data[predictions.ColumnCount() - 1].GetData();

    auto neg = (float *) predictions.data[0].GetData();
    auto pos = (float *) predictions.data[1].GetData();

    for (size_t i = 0; i < input.size(); i++) {
        // std::cout << "label: " << *lhs << std::endl;
        // std::cout << "pred: " << *rhs << std::endl;
        // std::cout << "match: " << (*lhs == static_cast<int>(*rhs)) << std::endl;
        // std::cout << "------" << std::endl;
        if ((*pos > *neg && *lhs > 0) || (*pos < *neg && *lhs < 1)) {
        // if (*lhs == static_cast<int>(*rhs)) {
            positives++;
        }
        lhs++;
        // rhs++;
        neg++;
        pos++;
    }

    state.stats.correct += positives;
    state.stats.total += input.size();
    // std::cout << "Pos = " << positives << ", Total = " << input.size() << ", Accuracy = " << positives * 1.0 / input.size() << std::endl;
        
    chunk.Fuse(predictions);
	return OperatorResultType::NEED_MORE_INPUT;
}

string PhysicalPredict::GetName() const {
    return "PREDICT";
}

string PhysicalPredict::ParamsToString() const {
    return model_name;
}

} // namespace duckdb
