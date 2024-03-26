#include "duckdb/planner/tableref/bound_predictref.hpp"
#include "duckdb/planner/operator/logical_predict.hpp"

namespace duckdb {

unique_ptr<LogicalOperator> Binder::CreatePlan(BoundPredictRef &ref) {
    auto subquery = ref.child_binder->CreatePlan(*ref.child);

	auto result = make_uniq<LogicalPredict>(ref.bind_index, std::move(subquery), std::move(ref.bound_predict));
	return std::move(result);
}

} // namespace duckdb
