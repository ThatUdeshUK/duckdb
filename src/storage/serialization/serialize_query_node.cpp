//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/parser/query_node/list.hpp"

namespace duckdb {

void QueryNode::Serialize(Serializer &serializer) const {
	if (serializer.ShouldSerialize(1)) {
		serializer.WriteProperty<QueryNodeType>(100, "type", type);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<vector<unique_ptr<ResultModifier>>>(101, "modifiers", modifiers);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WriteProperty<CommonTableExpressionMap>(102, "cte_map", cte_map);
	}
}

unique_ptr<QueryNode> QueryNode::Deserialize(Deserializer &deserializer) {
	auto type = deserializer.ReadProperty<QueryNodeType>(100, "type");
	auto modifiers = deserializer.ReadPropertyWithDefault<vector<unique_ptr<ResultModifier>>>(101, "modifiers");
	auto cte_map = deserializer.ReadProperty<CommonTableExpressionMap>(102, "cte_map");
	unique_ptr<QueryNode> result;
	switch (type) {
	case QueryNodeType::CTE_NODE:
		result = CTENode::Deserialize(deserializer);
		break;
	case QueryNodeType::RECURSIVE_CTE_NODE:
		result = RecursiveCTENode::Deserialize(deserializer);
		break;
	case QueryNodeType::SELECT_NODE:
		result = SelectNode::Deserialize(deserializer);
		break;
	case QueryNodeType::SET_OPERATION_NODE:
		result = SetOperationNode::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of QueryNode!");
	}
	result->modifiers = std::move(modifiers);
	result->cte_map = std::move(cte_map);
	return result;
}

void CTENode::Serialize(Serializer &serializer) const {
	QueryNode::Serialize(serializer);
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<string>(200, "cte_name", ctename);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<QueryNode>>(201, "query", query);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<QueryNode>>(202, "child", child);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<vector<string>>(203, "aliases", aliases);
	}
}

unique_ptr<QueryNode> CTENode::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CTENode>(new CTENode());
	deserializer.ReadPropertyWithDefault<string>(200, "cte_name", result->ctename);
	deserializer.ReadPropertyWithDefault<unique_ptr<QueryNode>>(201, "query", result->query);
	deserializer.ReadPropertyWithDefault<unique_ptr<QueryNode>>(202, "child", result->child);
	deserializer.ReadPropertyWithDefault<vector<string>>(203, "aliases", result->aliases);
	return std::move(result);
}

void RecursiveCTENode::Serialize(Serializer &serializer) const {
	QueryNode::Serialize(serializer);
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<string>(200, "cte_name", ctename);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<bool>(201, "union_all", union_all, false);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<QueryNode>>(202, "left", left);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<QueryNode>>(203, "right", right);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<vector<string>>(204, "aliases", aliases);
	}
}

unique_ptr<QueryNode> RecursiveCTENode::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<RecursiveCTENode>(new RecursiveCTENode());
	deserializer.ReadPropertyWithDefault<string>(200, "cte_name", result->ctename);
	deserializer.ReadPropertyWithDefault<bool>(201, "union_all", result->union_all, false);
	deserializer.ReadPropertyWithDefault<unique_ptr<QueryNode>>(202, "left", result->left);
	deserializer.ReadPropertyWithDefault<unique_ptr<QueryNode>>(203, "right", result->right);
	deserializer.ReadPropertyWithDefault<vector<string>>(204, "aliases", result->aliases);
	return std::move(result);
}

void SelectNode::Serialize(Serializer &serializer) const {
	QueryNode::Serialize(serializer);
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(200, "select_list", select_list);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<TableRef>>(201, "from_table", from_table);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(202, "where_clause", where_clause);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(203, "group_expressions", groups.group_expressions);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<vector<GroupingSet>>(204, "group_sets", groups.grouping_sets);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WriteProperty<AggregateHandling>(205, "aggregate_handling", aggregate_handling);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(206, "having", having);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<SampleOptions>>(207, "sample", sample);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(208, "qualify", qualify);
	}
}

unique_ptr<QueryNode> SelectNode::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<SelectNode>(new SelectNode());
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(200, "select_list", result->select_list);
	deserializer.ReadPropertyWithDefault<unique_ptr<TableRef>>(201, "from_table", result->from_table);
	deserializer.ReadPropertyWithDefault<unique_ptr<ParsedExpression>>(202, "where_clause", result->where_clause);
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(203, "group_expressions", result->groups.group_expressions);
	deserializer.ReadPropertyWithDefault<vector<GroupingSet>>(204, "group_sets", result->groups.grouping_sets);
	deserializer.ReadProperty<AggregateHandling>(205, "aggregate_handling", result->aggregate_handling);
	deserializer.ReadPropertyWithDefault<unique_ptr<ParsedExpression>>(206, "having", result->having);
	deserializer.ReadPropertyWithDefault<unique_ptr<SampleOptions>>(207, "sample", result->sample);
	deserializer.ReadPropertyWithDefault<unique_ptr<ParsedExpression>>(208, "qualify", result->qualify);
	return std::move(result);
}

void SetOperationNode::Serialize(Serializer &serializer) const {
	QueryNode::Serialize(serializer);
	if (serializer.ShouldSerialize(1)) {
		serializer.WriteProperty<SetOperationType>(200, "setop_type", setop_type);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<QueryNode>>(201, "left", left);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<unique_ptr<QueryNode>>(202, "right", right);
	}
	if (serializer.ShouldSerialize(1)) {
		serializer.WritePropertyWithDefault<bool>(203, "setop_all", setop_all, true);
	}
}

unique_ptr<QueryNode> SetOperationNode::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<SetOperationNode>(new SetOperationNode());
	deserializer.ReadProperty<SetOperationType>(200, "setop_type", result->setop_type);
	deserializer.ReadPropertyWithDefault<unique_ptr<QueryNode>>(201, "left", result->left);
	deserializer.ReadPropertyWithDefault<unique_ptr<QueryNode>>(202, "right", result->right);
	deserializer.ReadPropertyWithDefault<bool>(203, "setop_all", result->setop_all, true);
	return std::move(result);
}

} // namespace duckdb
