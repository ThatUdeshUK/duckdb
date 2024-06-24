//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/projection/physical_predict.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/main/client_config.hpp"
#include "duckdb/common/common.hpp"
#include "duckdb/planner/expression.hpp"

namespace duckdb {
enum class PredictorTask : uint8_t {
    PREDICT_TABULAR_TASK = 0,
    PREDICT_LLM_TASK = 1
};

struct PredictStats {
    long load;
    long preprocess;
    long move;
    long predict;
    long move_rev;

    long correct;
    long total;
};

class Predictor {
public:
    Predictor() : success(false), error_message(""){};
    virtual ~Predictor() {};

    PredictorTask task;
    std::string model_path;
    int batch_size;
    int llm_max_tokens; // LLM specific

    bool success;
    std::string error_message;
public:
    virtual void Config(const ClientConfig &config) {};
    virtual void Load(const std::string &model_path, PredictStats &stats) {};
    virtual void Predict(std::vector<float> &input, std::vector<float> &output, int output_size) {};
    virtual void PredictLM(std::string &input, std::vector<float> &output, int output_size) {};
    virtual void PredictLMChunk(DataChunk &input, DataChunk &output, int rows, const std::vector<idx_t> &input_mask, int output_size, PredictStats &stats) {};
    virtual void PredictVector(std::vector<float> &input, std::vector<float> &output, int rows, int cols, int output_size) {};
    virtual void PredictChunk(DataChunk &input, DataChunk &output, int rows, const std::vector<idx_t> &input_mask, int output_size, PredictStats &stats) {};
};

//! PhysicalPredict implements the physical PREDICT operation
class PhysicalPredict : public PhysicalOperator {
public:
    PhysicalPredict(vector<LogicalType> types, unique_ptr<PhysicalOperator> child);

    string model_name;

    std::vector<idx_t> input_mask;
    std::vector<LogicalType> result_set_types;

public:
    string GetName() const override;
    string ParamsToString() const override;

public:
    unique_ptr<OperatorState> GetOperatorState(ExecutionContext &context) const override;
    OperatorResultType Execute(ExecutionContext &context, DataChunk &input, DataChunk &chunk,
                               GlobalOperatorState &gstate, OperatorState &state_p) const override;

    bool ParallelOperator() const override {
        return true;
    }
private:
    unique_ptr<Predictor> InitPredictor() const;
};

} // namespace duckdb
