//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/projection/physical_predict.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/planner/expression.hpp"

namespace duckdb {
struct PredictStats {
    long load;
    long move;
    long predict;
    long move_rev;
};

class Predictor {
public:
    Predictor() : success(false), error_message(""){};
    virtual ~Predictor() {};

    bool success;
    std::string error_message;
public:
    virtual void Load(const std::string &model_path, PredictStats &stats) {};
    virtual void Predict(std::vector<float> &input, std::vector<float> &output, int output_size) {};
    virtual void PredictVector(std::vector<float> &input, std::vector<float> &output, int m, int n, int output_size) {};
    virtual void PredictChunk(DataChunk &input, DataChunk &output, int m, int n, int output_size, PredictStats &stats) {};
};

//! PhysicalPredict implements the physical PREDICT operation
class PhysicalPredict : public PhysicalOperator {
public:
    PhysicalPredict(vector<LogicalType> types, unique_ptr<PhysicalOperator> child);

    string model_name;

//    vector<string> result_set_names;
    vector<LogicalType> result_set_types;

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
