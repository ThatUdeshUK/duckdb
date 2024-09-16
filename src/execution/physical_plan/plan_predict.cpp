#include "duckdb/execution/operator/projection/physical_predict.hpp"
#include "duckdb/execution/physical_plan_generator.hpp"
#include "duckdb/planner/operator/logical_predict.hpp"

namespace duckdb {

unique_ptr<PhysicalOperator> PhysicalPlanGenerator::CreatePlan(LogicalPredict &op) {
	if (op.bound_predict.model_type != 2) {
		D_ASSERT(op.children.size() == 1);
		auto child_plan = CreatePlan(*op.children[0]);
		auto predict = make_uniq<PhysicalPredict>(std::move(op.types), std::move(child_plan));
		predict->model_type = std::move(op.bound_predict.model_type);
		predict->model_name = std::move(op.bound_predict.model_name);
		predict->input_mask = std::move(op.bound_predict.input_mask);
		predict->result_set_types = std::move(op.bound_predict.result_set_types);
		return std::move(predict);
	} else {
		D_ASSERT(op.children.size() == 2);
		idx_t node_cardinality = op.children[0]->EstimateCardinality(context);
		idx_t edge_cardinality = op.children[1]->EstimateCardinality(context);
		auto node_plan = CreatePlan(*op.children[0]);
		auto edge_plan = CreatePlan(*op.children[1]);
		auto predict = make_uniq<PhysicalGNNPredict>(std::move(op.types), node_cardinality);
		predict->children.push_back(std::move(node_plan));
		predict->children.push_back(std::move(edge_plan));
		predict->model_type = std::move(op.bound_predict.model_type);
		predict->model_name = std::move(op.bound_predict.model_name);
		predict->num_nodes = node_cardinality;
		predict->num_edges = edge_cardinality;
		predict->node_mask = std::move(op.bound_predict.input_mask);
		predict->edge_mask = std::move(op.bound_predict.opt_mask);
		predict->node_types = std::move(op.bound_predict.input_set_types);
		predict->result_set_types = std::move(op.bound_predict.result_set_types);
		return std::move(predict);
	}
}

} // namespace duckdb
