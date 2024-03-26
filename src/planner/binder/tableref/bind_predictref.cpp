#include "duckdb/parser/tableref/predictref.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/tableref/bound_predictref.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"
#include "duckdb/parser/query_node/select_node.hpp"

namespace duckdb {

unique_ptr<BoundTableRef> Binder::BindBoundPredict(PredictRef &ref) {
    auto result = make_uniq<BoundPredictRef>();
    result->bind_index = GenerateTableIndex();
    result->child_binder = Binder::CreateBinder(context, this);
    result->child = result->child_binder->Bind(*ref.source);

    result->bound_predict.model_name = ref.model_name;

    vector<string> names;
    vector<LogicalType> types;
    result->child_binder->bind_context.GetTypesAndNames(names, types);

    vector<string> result_names = ref.result_set_names;
    names.insert(names.end(), std::make_move_iterator(result_names.begin()), std::make_move_iterator(result_names.end()));

    vector<LogicalType> result_types = ref.result_set_types;
    types.insert(types.end(), std::make_move_iterator(result_types.begin()), std::make_move_iterator(result_types.end()));

    result->bound_predict.types = types;
    result->bound_predict.result_set_names = std::move(ref.result_set_names);
    result->bound_predict.result_set_types = std::move(ref.result_set_types);

    auto subquery_alias = ref.alias.empty() ? "__unnamed_predict" : ref.alias;
    bind_context.AddGenericBinding(result->bind_index, subquery_alias, names, types);
    MoveCorrelatedExpressions(*result->child_binder);
    return std::move(result);
}

unique_ptr<BoundTableRef> Binder::Bind(PredictRef &ref) {
    if (!ref.source) {
        throw InternalException("Predict without a source!?");
    }

    // Wrap the source in a projection
    auto subquery = make_uniq<SelectNode>();
    subquery->select_list.push_back(make_uniq<StarExpression>());
    subquery->from_table = std::move(ref.source);

    auto subquery_select = make_uniq<SelectStatement>();
    subquery_select->node = std::move(subquery);
    auto subquery_ref = make_uniq<SubqueryRef>(std::move(subquery_select));

    ref.source = std::move(subquery_ref);

    return BindBoundPredict(ref);
}

} // namespace duckdb
