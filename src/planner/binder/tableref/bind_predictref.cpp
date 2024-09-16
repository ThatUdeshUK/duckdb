#include "duckdb/parser/tableref/predictref.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/tableref/bound_predictref.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"
#include "duckdb/parser/query_node/select_node.hpp"

namespace duckdb {

unique_ptr<BoundTableRef> Binder::BindBoundPredict(PredictRef &ref) {
    auto result = make_uniq<BoundPredictRef>();
    result->bound_predict.model_type = ref.model_type;
    result->bound_predict.model_name = ref.model_name;
    result->bind_index = GenerateTableIndex();
    result->child_binder = Binder::CreateBinder(context, this);
    result->children.push_back(result->child_binder->Bind(*ref.source));

    vector<string> names;
    vector<LogicalType> input_types;
    result->child_binder->bind_context.GetTypesAndNames(names, input_types);
    vector<LogicalType> types(input_types);

    vector<idx_t> input_mask;
    if (ref.input_set_names.size() > 0) {
        for(const std::string& input_col : ref.input_set_names) {
            bool feature_found = false;
            for (auto it = names.begin(); it != names.end(); ++it) {
                int index = std::distance(names.begin(), it);
                if (input_col == *it) {
                    input_mask.push_back(index);
                    feature_found = true;
                    break;
                }
            }
            if (!feature_found) {
                throw BinderException("Input tabel should contain the BY feature columns");
            }
        }
    } else if (ref.exclude_set_names.size() > 0) {
        for (auto it = names.begin(); it != names.end(); ++it) {
            bool exclude_found = false;
            for(const std::string& exclude_col : ref.exclude_set_names) {
                if (exclude_col == *it) {
                    exclude_found = true;
                    break;
                }
            }
            if (exclude_found) {
                continue;
            }
            int index = std::distance(names.begin(), it);
            input_mask.push_back(index);
        }
    } else {
        for (int i = 0; i < names.size(); i++) {
            input_mask.push_back(i);
        }
    }
    result->bound_predict.input_mask = input_mask;

    if (ref.model_type == 2) {
        vector<string> opt_names;
        vector<LogicalType> opt_types;
        result->opt_binder = Binder::CreateBinder(context, this);
        result->children.push_back(result->opt_binder->Bind(*ref.opt_source));

        result->opt_binder->bind_context.GetTypesAndNames(opt_names, opt_types);

        vector<idx_t> opt_mask;
        if (ref.opt_set_names.size() > 0) {
            for(const std::string& input_col : ref.opt_set_names) {
                bool feature_found = false;
                for (auto it = opt_names.begin(); it != opt_names.end(); ++it) {
                    int index = std::distance(opt_names.begin(), it);
                    if (input_col == *it) {
                        opt_mask.push_back(index);
                        feature_found = true;
                        break;
                    }
                }
                if (!feature_found) {
                    throw BinderException("Input tabel should contain the BY feature columns");
                }
            }
        } else if (ref.exclude_opt_set_names.size() > 0) {
            for (auto it = opt_names.begin(); it != opt_names.end(); ++it) {
                bool exclude_found = false;
                for(const std::string& exclude_col : ref.exclude_opt_set_names) {
                    if (exclude_col == *it) {
                        exclude_found = true;
                        break;
                    }
                }
                if (exclude_found) {
                    continue;
                }
                int index = std::distance(opt_names.begin(), it);
                opt_mask.push_back(index);
            }
        } else {
            for (int i = 0; i < opt_names.size(); i++) {
                opt_mask.push_back(i);
            }
        }

        // names.insert(names.end(), std::make_move_iterator(opt_names.begin()), std::make_move_iterator(opt_names.end()));
        // types.insert(types.end(), std::make_move_iterator(opt_types.begin()), std::make_move_iterator(opt_types.end()));
        
        result->bound_predict.opt_mask = opt_mask;
    }

    vector<string> result_names = ref.result_set_names;
    names.insert(names.end(), std::make_move_iterator(result_names.begin()), std::make_move_iterator(result_names.end()));

    vector<LogicalType> result_types = ref.result_set_types;
    types.insert(types.end(), std::make_move_iterator(result_types.begin()), std::make_move_iterator(result_types.end()));

    result->bound_predict.types = types;
    result->bound_predict.result_set_names = std::move(ref.result_set_names);
    result->bound_predict.input_set_types = std::move(input_types);
    result->bound_predict.result_set_types = std::move(ref.result_set_types);

    auto subquery_alias = ref.alias.empty() ? "__unnamed_predict" : ref.alias;
    bind_context.AddGenericBinding(result->bind_index, subquery_alias, names, types);
    MoveCorrelatedExpressions(*result->child_binder);
    // MoveCorrelatedExpressions(*result->opt_binder);
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
