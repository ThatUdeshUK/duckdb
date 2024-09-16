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
    OperatorResultType Execute(ExecutionContext &context, DataChunk &input, DataChunk &chunk,
                               GlobalOperatorState &gstate, OperatorState &state) const override;

    bool ParallelOperator() const override {
        return true;
    }
};

} // namespace duckdb
