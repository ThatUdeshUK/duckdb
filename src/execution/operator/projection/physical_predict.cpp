#include "duckdb/execution/operator/projection/physical_predict.hpp"
#include "duckdb/common/common.hpp"

namespace duckdb {

PhysicalPredict::PhysicalPredict(vector<LogicalType> types_p, unique_ptr<PhysicalOperator> child)
    : PhysicalOperator(PhysicalOperatorType::PREDICT, std::move(types_p), child->estimated_cardinality) {
	children.push_back(std::move(child));
}

OperatorResultType PhysicalPredict::Execute(ExecutionContext &context, DataChunk &input, DataChunk &chunk,
                                          GlobalOperatorState &gstate, OperatorState &state) const {

    DataChunk predictions;
    predictions.Initialize(Allocator::Get(context.client), result_set_types);
    predictions.SetCardinality(input);
    for (idx_t row_idx = 0; row_idx < input.size(); row_idx++) {
        for (idx_t col_idx = 0; col_idx < result_set_types.size(); col_idx++) {
            predictions.SetValue(col_idx, row_idx, Value());
        }
    }

    input.Fuse(predictions);
    chunk.Move(input);
//
//    idx_t column_count = chunk.ColumnCount();
//    for (idx_t i = 0; i < chunk.size(); ++i) {
//        auto value = chunk.GetValue(column_count - 1, i);
//        chunk.SetValue(column_count - 1, i, Value::BLOB("hello"));
//    }

	return OperatorResultType::NEED_MORE_INPUT;
}

string PhysicalPredict::GetName() const {
    return "PREDICT";
}

string PhysicalPredict::ParamsToString() const {
    return model_name;
}

} // namespace duckdb
