#include "duckdb/parser/statement/drop_statement.hpp"
#include "duckdb/main/secret/secret_manager.hpp"
#include "duckdb/parser/transformer.hpp"

namespace duckdb {

unique_ptr<SQLStatement> Transformer::TransformDrop(duckdb_libpgquery::PGDropStmt &stmt) {
	auto result = make_uniq<DropStatement>();
	auto &info = *result->info.get();
	if (stmt.objects->length != 1) {
		throw NotImplementedException("Can only drop one object at a time");
	}
	switch (stmt.removeType) {
	case duckdb_libpgquery::PG_OBJECT_TABLE:
		info.type = CatalogType::TABLE_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_SCHEMA:
		info.type = CatalogType::SCHEMA_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_INDEX:
		info.type = CatalogType::INDEX_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_VIEW:
		info.type = CatalogType::VIEW_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_SEQUENCE:
		info.type = CatalogType::SEQUENCE_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_MODEL:
		info.type = CatalogType::MODEL_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_FUNCTION:
		info.type = CatalogType::MACRO_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_TABLE_MACRO:
		info.type = CatalogType::TABLE_MACRO_ENTRY;
		break;
	case duckdb_libpgquery::PG_OBJECT_TYPE:
		info.type = CatalogType::TYPE_ENTRY;
		break;
	default:
		throw NotImplementedException("Cannot drop this type yet");
	}

	switch (stmt.removeType) {
	case duckdb_libpgquery::PG_OBJECT_SCHEMA: {
		auto view_list = PGPointerCast<duckdb_libpgquery::PGList>(stmt.objects->head->data.ptr_value);
		if (view_list->length == 2) {
			info.catalog = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->data.ptr_value)->val.str;
			info.name = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->next->data.ptr_value)->val.str;
		} else if (view_list->length == 1) {
			info.name = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->data.ptr_value)->val.str;
		} else {
			throw ParserException("Expected \"catalog.schema\" or \"schema\"");
		}
		break;
	}
	default: {
		auto view_list = PGPointerCast<duckdb_libpgquery::PGList>(stmt.objects->head->data.ptr_value);
		if (view_list->length == 3) {
			info.catalog = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->data.ptr_value)->val.str;
			info.schema = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->next->data.ptr_value)->val.str;
			info.name = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->next->next->data.ptr_value)->val.str;
		} else if (view_list->length == 2) {
			info.schema = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->data.ptr_value)->val.str;
			info.name = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->next->data.ptr_value)->val.str;
		} else if (view_list->length == 1) {
			info.name = PGPointerCast<duckdb_libpgquery::PGValue>(view_list->head->data.ptr_value)->val.str;
		} else {
			throw ParserException("Expected \"catalog.schema.name\", \"schema.name\"or \"name\"");
		}
		break;
	}
	}
	info.cascade = stmt.behavior == duckdb_libpgquery::PGDropBehavior::PG_DROP_CASCADE;
	info.if_not_found = TransformOnEntryNotFound(stmt.missing_ok);
	return std::move(result);
}

unique_ptr<DropStatement> Transformer::TransformDropSecret(duckdb_libpgquery::PGDropSecretStmt &stmt) {
	auto result = make_uniq<DropStatement>();
	auto info = make_uniq<DropInfo>();
	auto extra_info = make_uniq<ExtraDropSecretInfo>();

	info->type = CatalogType::SECRET_ENTRY;
	info->name = stmt.secret_name;
	info->if_not_found = stmt.missing_ok ? OnEntryNotFound::RETURN_NULL : OnEntryNotFound::THROW_EXCEPTION;

	extra_info->persist_mode = EnumUtil::FromString<SecretPersistType>(StringUtil::Upper(stmt.persist_type));
	extra_info->secret_storage = stmt.secret_storage;

	if (extra_info->persist_mode == SecretPersistType::TEMPORARY) {
		if (!extra_info->secret_storage.empty()) {
			throw ParserException("Can not combine TEMPORARY with specifying a storage for drop secret");
		}
	}

	info->extra_drop_info = std::move(extra_info);
	result->info = std::move(info);

	return result;
}

} // namespace duckdb
