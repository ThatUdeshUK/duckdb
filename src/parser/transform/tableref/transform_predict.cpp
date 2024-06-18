#include "duckdb/parser/tableref/predictref.hpp"
#include "duckdb/parser/transformer.hpp"

namespace duckdb {

unique_ptr<TableRef> Transformer::TransformPredict(duckdb_libpgquery::PGPredictExpr &root) {
	auto result = make_uniq<PredictRef>();
	result->source = TransformTableRefNode(*root.source);
    result->model_name = root.model_name;
	result->alias = TransformAlias(root.alias, result->column_name_alias);

	for (auto c = root.input_set->head; c != nullptr; c = lnext(c)) {
		result->input_set_names.push_back(reinterpret_cast<duckdb_libpgquery::PGValue *>(c->data.ptr_value)->val.str);
	}

    for (auto c = root.result_set->head; c != nullptr; c = lnext(c)) {
        auto cdef = PGPointerCast<duckdb_libpgquery::PGColumnDef>(c->data.ptr_value);
        auto centry = TransformColumnDefinition(*cdef);
        if (cdef->constraints) {
            throw ParserException("Result set must not contain constraints!");
        }
        result->result_set_names.push_back(centry.GetName());
        result->result_set_types.push_back(centry.GetType());
    }

	SetQueryLocation(*result, root.location);
	return std::move(result);
}

} // namespace duckdb
