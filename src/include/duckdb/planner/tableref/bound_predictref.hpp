//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/planner/tableref/bound_predictref.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/bound_tableref.hpp"

namespace duckdb {

struct BoundPredictInfo {
    string model_name;
    //! The set of types
    vector<LogicalType> types;
    //! Result set names
    vector<string> result_set_names;
    //! Result set types
    vector<LogicalType> result_set_types;

    void Serialize(Serializer &serializer) const;
    static BoundPredictInfo Deserialize(Deserializer &deserializer);
};

class BoundPredictRef : public BoundTableRef {
public:
	static constexpr const TableReferenceType TYPE = TableReferenceType::PREDICT;

public:
	explicit BoundPredictRef() : BoundTableRef(TableReferenceType::PREDICT) {
	}

    idx_t bind_index;
    //! The binder used to bind the child of the pivot
    shared_ptr<Binder> child_binder;
	//! The child node of the Predict
	unique_ptr<BoundTableRef> child;
    //! The bound predict info
    BoundPredictInfo bound_predict;
};
} // namespace duckdb
