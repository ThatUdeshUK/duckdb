//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/parser/parsed_data/create_info.hpp"
#include "duckdb/parser/parsed_data/create_index_info.hpp"
#include "duckdb/parser/parsed_data/create_table_info.hpp"
#include "duckdb/parser/parsed_data/create_schema_info.hpp"
#include "duckdb/parser/parsed_data/create_view_info.hpp"
#include "duckdb/parser/parsed_data/create_type_info.hpp"
#include "duckdb/parser/parsed_data/create_macro_info.hpp"
#include "duckdb/parser/parsed_data/create_sequence_info.hpp"
#include "duckdb/parser/parsed_data/create_model_info.hpp"

namespace duckdb {

void CreateInfo::Serialize(Serializer &serializer) const {
	serializer.WriteProperty<CatalogType>(100, "type", type);
	serializer.WritePropertyWithDefault<string>(101, "catalog", catalog);
	serializer.WritePropertyWithDefault<string>(102, "schema", schema);
	serializer.WritePropertyWithDefault<bool>(103, "temporary", temporary);
	serializer.WritePropertyWithDefault<bool>(104, "internal", internal);
	serializer.WriteProperty<OnCreateConflict>(105, "on_conflict", on_conflict);
	serializer.WritePropertyWithDefault<string>(106, "sql", sql);
	serializer.WritePropertyWithDefault<Value>(107, "comment", comment, Value());
	serializer.WritePropertyWithDefault<unordered_map<string, string>>(108, "tags", tags, unordered_map<string, string>());
	if (serializer.ShouldSerialize(2)) {
		serializer.WritePropertyWithDefault<LogicalDependencyList>(109, "dependencies", dependencies, LogicalDependencyList());
	}
}

unique_ptr<CreateInfo> CreateInfo::Deserialize(Deserializer &deserializer) {
	auto type = deserializer.ReadProperty<CatalogType>(100, "type");
	auto catalog = deserializer.ReadPropertyWithDefault<string>(101, "catalog");
	auto schema = deserializer.ReadPropertyWithDefault<string>(102, "schema");
	auto temporary = deserializer.ReadPropertyWithDefault<bool>(103, "temporary");
	auto internal = deserializer.ReadPropertyWithDefault<bool>(104, "internal");
	auto on_conflict = deserializer.ReadProperty<OnCreateConflict>(105, "on_conflict");
	auto sql = deserializer.ReadPropertyWithDefault<string>(106, "sql");
	auto comment = deserializer.ReadPropertyWithExplicitDefault<Value>(107, "comment", Value());
	auto tags = deserializer.ReadPropertyWithExplicitDefault<unordered_map<string, string>>(108, "tags", unordered_map<string, string>());
	auto dependencies = deserializer.ReadPropertyWithExplicitDefault<LogicalDependencyList>(109, "dependencies", LogicalDependencyList());
	deserializer.Set<CatalogType>(type);
	unique_ptr<CreateInfo> result;
	switch (type) {
	case CatalogType::INDEX_ENTRY:
		result = CreateIndexInfo::Deserialize(deserializer);
		break;
	case CatalogType::MACRO_ENTRY:
		result = CreateMacroInfo::Deserialize(deserializer);
		break;
	case CatalogType::SCHEMA_ENTRY:
		result = CreateSchemaInfo::Deserialize(deserializer);
		break;
	case CatalogType::SEQUENCE_ENTRY:
		result = CreateSequenceInfo::Deserialize(deserializer);
		break;
	case CatalogType::MODEL_ENTRY:
		result = CreateModelInfo::Deserialize(deserializer);
		break;
	case CatalogType::TABLE_ENTRY:
		result = CreateTableInfo::Deserialize(deserializer);
		break;
	case CatalogType::TABLE_MACRO_ENTRY:
		result = CreateMacroInfo::Deserialize(deserializer);
		break;
	case CatalogType::TYPE_ENTRY:
		result = CreateTypeInfo::Deserialize(deserializer);
		break;
	case CatalogType::VIEW_ENTRY:
		result = CreateViewInfo::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of CreateInfo!");
	}
	deserializer.Unset<CatalogType>();
	result->catalog = std::move(catalog);
	result->schema = std::move(schema);
	result->temporary = temporary;
	result->internal = internal;
	result->on_conflict = on_conflict;
	result->sql = std::move(sql);
	result->comment = comment;
	result->tags = std::move(tags);
	result->dependencies = dependencies;
	return result;
}

void CreateIndexInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", index_name);
	serializer.WritePropertyWithDefault<string>(201, "table", table);
	/* [Deleted] (DeprecatedIndexType) "index_type" */
	serializer.WriteProperty<IndexConstraintType>(203, "constraint_type", constraint_type);
	serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(204, "parsed_expressions", parsed_expressions);
	serializer.WritePropertyWithDefault<vector<LogicalType>>(205, "scan_types", scan_types);
	serializer.WritePropertyWithDefault<vector<string>>(206, "names", names);
	serializer.WritePropertyWithDefault<vector<column_t>>(207, "column_ids", column_ids);
	serializer.WritePropertyWithDefault<case_insensitive_map_t<Value>>(208, "options", options);
	serializer.WritePropertyWithDefault<string>(209, "index_type_name", index_type);
}

unique_ptr<CreateInfo> CreateIndexInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateIndexInfo>(new CreateIndexInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "name", result->index_name);
	deserializer.ReadPropertyWithDefault<string>(201, "table", result->table);
	deserializer.ReadDeletedProperty<DeprecatedIndexType>(202, "index_type");
	deserializer.ReadProperty<IndexConstraintType>(203, "constraint_type", result->constraint_type);
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(204, "parsed_expressions", result->parsed_expressions);
	deserializer.ReadPropertyWithDefault<vector<LogicalType>>(205, "scan_types", result->scan_types);
	deserializer.ReadPropertyWithDefault<vector<string>>(206, "names", result->names);
	deserializer.ReadPropertyWithDefault<vector<column_t>>(207, "column_ids", result->column_ids);
	deserializer.ReadPropertyWithDefault<case_insensitive_map_t<Value>>(208, "options", result->options);
	deserializer.ReadPropertyWithDefault<string>(209, "index_type_name", result->index_type);
	return std::move(result);
}

void CreateMacroInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", name);
	serializer.WritePropertyWithDefault<unique_ptr<MacroFunction>>(201, "function", macros[0]);
	serializer.WritePropertyWithDefault<vector<unique_ptr<MacroFunction>>>(202, "extra_functions", GetAllButFirstFunction());
}

unique_ptr<CreateInfo> CreateMacroInfo::Deserialize(Deserializer &deserializer) {
	auto name = deserializer.ReadPropertyWithDefault<string>(200, "name");
	auto function = deserializer.ReadPropertyWithDefault<unique_ptr<MacroFunction>>(201, "function");
	auto extra_functions = deserializer.ReadPropertyWithDefault<vector<unique_ptr<MacroFunction>>>(202, "extra_functions");
	auto result = duckdb::unique_ptr<CreateMacroInfo>(new CreateMacroInfo(deserializer.Get<CatalogType>(), std::move(function), std::move(extra_functions)));
	result->name = std::move(name);
	return std::move(result);
}

void CreateSchemaInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
}

unique_ptr<CreateInfo> CreateSchemaInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateSchemaInfo>(new CreateSchemaInfo());
	return std::move(result);
}

void CreateSequenceInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", name);
	serializer.WritePropertyWithDefault<uint64_t>(201, "usage_count", usage_count);
	serializer.WritePropertyWithDefault<int64_t>(202, "increment", increment);
	serializer.WritePropertyWithDefault<int64_t>(203, "min_value", min_value);
	serializer.WritePropertyWithDefault<int64_t>(204, "max_value", max_value);
	serializer.WritePropertyWithDefault<int64_t>(205, "start_value", start_value);
	serializer.WritePropertyWithDefault<bool>(206, "cycle", cycle);
}

unique_ptr<CreateInfo> CreateSequenceInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateSequenceInfo>(new CreateSequenceInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "name", result->name);
	deserializer.ReadPropertyWithDefault<uint64_t>(201, "usage_count", result->usage_count);
	deserializer.ReadPropertyWithDefault<int64_t>(202, "increment", result->increment);
	deserializer.ReadPropertyWithDefault<int64_t>(203, "min_value", result->min_value);
	deserializer.ReadPropertyWithDefault<int64_t>(204, "max_value", result->max_value);
	deserializer.ReadPropertyWithDefault<int64_t>(205, "start_value", result->start_value);
	deserializer.ReadPropertyWithDefault<bool>(206, "cycle", result->cycle);
	return std::move(result);
}


void CreateModelInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", name);
	serializer.WritePropertyWithDefault<uint8_t>(201, "model_type", model_type);
	serializer.WritePropertyWithDefault<string>(202, "model_path", model_path);
	serializer.WritePropertyWithDefault<string>(203, "rel_name", rel_name);
	serializer.WritePropertyWithDefault<vector<string>>(204, "input_set_names", input_set_names);
	serializer.WritePropertyWithDefault<vector<string>>(205, "exclude_set_names", exclude_set_names);
	serializer.WritePropertyWithDefault<string>(206, "opt_rel_name", opt_rel_name);
	serializer.WritePropertyWithDefault<vector<string>>(207, "opt_set_names", opt_set_names);
	serializer.WritePropertyWithDefault<vector<string>>(208, "exclude_opt_set_names", exclude_opt_set_names);
	serializer.WritePropertyWithDefault<vector<string>>(209, "out_names", out_names);
	serializer.WritePropertyWithDefault<vector<LogicalType>>(210, "out_types", out_types);
	serializer.WritePropertyWithDefault<case_insensitive_map_t<Value>>(211, "options", options);
}

unique_ptr<CreateInfo> CreateModelInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateModelInfo>(new CreateModelInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "name", result->name);
	deserializer.ReadPropertyWithDefault<uint8_t>(201, "model_type", result->model_type);
	deserializer.ReadPropertyWithDefault<string>(202, "model_path", result->model_path);
	deserializer.ReadPropertyWithDefault<string>(203, "rel_name", result->rel_name);
	deserializer.ReadPropertyWithDefault<vector<string>>(204, "input_set_names", result->input_set_names);
	deserializer.ReadPropertyWithDefault<vector<string>>(205, "exclude_set_names", result->exclude_set_names);
	deserializer.ReadPropertyWithDefault<string>(206, "opt_rel_name", result->opt_rel_name);
	deserializer.ReadPropertyWithDefault<vector<string>>(207, "opt_set_names", result->opt_set_names);
	deserializer.ReadPropertyWithDefault<vector<string>>(208, "exclude_opt_set_names", result->exclude_opt_set_names);
	deserializer.ReadPropertyWithDefault<vector<string>>(209, "out_names", result->out_names);
	deserializer.ReadPropertyWithDefault<vector<LogicalType>>(210, "out_types", result->out_types);
	deserializer.ReadPropertyWithDefault<case_insensitive_map_t<Value>>(211, "options", result->options);
	return std::move(result);
}


void CreateTableInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "table", table);
	serializer.WriteProperty<ColumnList>(201, "columns", columns);
	serializer.WritePropertyWithDefault<vector<unique_ptr<Constraint>>>(202, "constraints", constraints);
	serializer.WritePropertyWithDefault<unique_ptr<SelectStatement>>(203, "query", query);
}

unique_ptr<CreateInfo> CreateTableInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateTableInfo>(new CreateTableInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "table", result->table);
	deserializer.ReadProperty<ColumnList>(201, "columns", result->columns);
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<Constraint>>>(202, "constraints", result->constraints);
	deserializer.ReadPropertyWithDefault<unique_ptr<SelectStatement>>(203, "query", result->query);
	return std::move(result);
}

void CreateTypeInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "name", name);
	serializer.WriteProperty<LogicalType>(201, "logical_type", type);
}

unique_ptr<CreateInfo> CreateTypeInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateTypeInfo>(new CreateTypeInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "name", result->name);
	deserializer.ReadProperty<LogicalType>(201, "logical_type", result->type);
	return std::move(result);
}

void CreateViewInfo::Serialize(Serializer &serializer) const {
	CreateInfo::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "view_name", view_name);
	serializer.WritePropertyWithDefault<vector<string>>(201, "aliases", aliases);
	serializer.WritePropertyWithDefault<vector<LogicalType>>(202, "types", types);
	serializer.WritePropertyWithDefault<unique_ptr<SelectStatement>>(203, "query", query);
	serializer.WritePropertyWithDefault<vector<string>>(204, "names", names);
	serializer.WritePropertyWithDefault<vector<Value>>(205, "column_comments", column_comments, vector<Value>());
}

unique_ptr<CreateInfo> CreateViewInfo::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<CreateViewInfo>(new CreateViewInfo());
	deserializer.ReadPropertyWithDefault<string>(200, "view_name", result->view_name);
	deserializer.ReadPropertyWithDefault<vector<string>>(201, "aliases", result->aliases);
	deserializer.ReadPropertyWithDefault<vector<LogicalType>>(202, "types", result->types);
	deserializer.ReadPropertyWithDefault<unique_ptr<SelectStatement>>(203, "query", result->query);
	deserializer.ReadPropertyWithDefault<vector<string>>(204, "names", result->names);
	deserializer.ReadPropertyWithExplicitDefault<vector<Value>>(205, "column_comments", result->column_comments, vector<Value>());
	return std::move(result);
}

} // namespace duckdb
