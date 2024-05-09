#include "duckdb/execution/operator/projection/physical_predict.hpp"
#include "duckdb/common/common.hpp"

#define ZERO_COPY_VEC_PRED 1
#define VEC_PRED 0

namespace duckdb {
class PredictState : public OperatorState {
public:
    explicit PredictState(ExecutionContext &context, TorchPredictor predictor, TorchPredictStats stats) {
        this->predictor = predictor;
        this->stats = stats;
    }

    TorchPredictor predictor;
    TorchPredictStats stats;

public:
    void Finalize(const PhysicalOperator &op, ExecutionContext &context) override {
        std::cout << "Load @run: " << stats.load << std::endl;
        std::cout << "Move @run: " << stats.move << std::endl;
        std::cout << "Predict @run: " << stats.predict << std::endl;
        std::cout << "Move Rev @run: " << stats.move_rev << std::endl;

//        context.thread.profiler.Flush(op, executor, "projection", 0);
    }
};

PhysicalPredict::PhysicalPredict(vector<LogicalType> types_p, unique_ptr<PhysicalOperator> child)
    : PhysicalOperator(PhysicalOperatorType::PREDICT, std::move(types_p), child->estimated_cardinality) {
	children.push_back(std::move(child));
}

unique_ptr<OperatorState> PhysicalPredict::GetOperatorState(ExecutionContext &context) const {
    unique_ptr<TorchPredictStats> stats = make_uniq<TorchPredictStats>();
    unique_ptr<TorchPredictor> p = make_uniq<TorchPredictor>();
    p->Load(model_name, *stats);
    return make_uniq<PredictState>(context, *p, *stats);
}

OperatorResultType PhysicalPredict::Execute(ExecutionContext &context, DataChunk &input, DataChunk &chunk,
                                          GlobalOperatorState &gstate, OperatorState &state_p) const {
    auto &state = state_p.Cast<PredictState>();

    std::vector<float> inputs;
    std::vector<float> outputs;

    DataChunk predictions;
    chunk.Split(predictions, input.ColumnCount());
    input.Copy(chunk);

    predictions.SetCardinality(input);

#if ZERO_COPY_VEC_PRED
    state.predictor.PredictChunk(input, predictions, (int) input.size(), (int) input.ColumnCount(), (int) result_set_types.size(), state.stats);
#elif VEC_PRED
    for (idx_t row_idx = 0; row_idx < input.size(); row_idx++) {
        for (idx_t col_idx = 0; col_idx < input.ColumnCount(); col_idx++) {
            inputs.push_back(FloatValue::Get(input.GetValue(col_idx, row_idx)));
        }
    }
    state.predictor.PredictVector(inputs, outputs, (int) input.size(), (int) input.ColumnCount(), (int) result_set_types.size());
    int col_count = (int) result_set_types.size();
    idx_t idx = 0;
    for(const float& i : outputs) {
        predictions.SetValue(idx % col_count, idx / col_count, Value(i));
        idx++;
    }
    inputs.clear();
    outputs.clear();
#else
    for (idx_t row_idx = 0; row_idx < input.size(); row_idx++) {
        for (idx_t col_idx = 0; col_idx < input.ColumnCount(); col_idx++) {
            inputs.push_back(FloatValue::Get(input.GetValue(col_idx, row_idx)));
        }
        state.predictor.Predict(inputs, outputs, (int) result_set_types.size());
        idx_t res_idx = 0;
        for(const float& i : outputs) {
            predictions.SetValue(res_idx, row_idx, Value(i));
            res_idx++;
        }

        inputs.clear();
        outputs.clear();
    }
#endif // VECTORIZED_PREDICTIONS

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
