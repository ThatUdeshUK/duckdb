#include "duckdb/parser/tableref/predictref.hpp"
#include "duckdb/common/limits.hpp"

namespace duckdb {

//===--------------------------------------------------------------------===//
// PredictRef
//===--------------------------------------------------------------------===//
string PredictRef::ToString() const {
	string result;
	result = source->ToString();
	result += " PREDICT USING " + model_name;
	return result;
}

bool PredictRef::Equals(const TableRef &other_p) const {
	if (!TableRef::Equals(other_p)) {
		return false;
	}
	auto &other = other_p.Cast<PredictRef>();
	if (!source->Equals(*other.source)) {
		return false;
	}
	if (model_name != other.model_name) {
		return false;
	}
	if (alias != other.alias) {
		return false;
	}
	return true;
}

unique_ptr<TableRef> PredictRef::Copy() {
	auto copy = make_uniq<PredictRef>();
	copy->source = source->Copy();
	copy->model_name = model_name;
	copy->alias = alias;
	return std::move(copy);
}

} // namespace duckdb
