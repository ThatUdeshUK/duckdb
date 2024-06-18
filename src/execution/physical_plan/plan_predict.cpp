#include "duckdb/execution/operator/projection/physical_predict.hpp"
#include "duckdb/execution/physical_plan_generator.hpp"
#include "duckdb/planner/operator/logical_predict.hpp"

namespace duckdb {

unique_ptr<PhysicalOperator> PhysicalPlanGenerator::CreatePlan(LogicalPredict &op) {
	D_ASSERT(op.children.size() == 1);
	auto child_plan = CreatePlan(*op.children[0]);
	auto predict = make_uniq<PhysicalPredict>(std::move(op.types), std::move(child_plan));
	predict->model_name = op.bound_predict.model_name;
   	predict->input_mask = std::move(op.bound_predict.input_mask);
    predict->result_set_types = std::move(op.bound_predict.result_set_types);
    return std::move(predict);
}

} // namespace duckdb
