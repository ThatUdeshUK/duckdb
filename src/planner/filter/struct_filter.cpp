#include "duckdb/planner/filter/struct_filter.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "duckdb/storage/statistics/struct_stats.hpp"

namespace duckdb {

StructFilter::StructFilter(idx_t child_idx_p, const string &child_name_p, unique_ptr<TableFilter> child_filter_p)
    : TableFilter(TableFilterType::STRUCT_EXTRACT), child_idx(child_idx_p), child_name(child_name_p),
      child_filter(std::move(child_filter_p)) {
}

FilterPropagateResult StructFilter::CheckStatistics(BaseStatistics &stats) {
	auto &type = stats.GetType();
	D_ASSERT(type.id() == LogicalTypeId::STRUCT);
	// Check the child statistics
	auto &child_stats = StructStats::GetChildStats(stats, child_idx);
	return child_filter->CheckStatistics(child_stats);
}

string StructFilter::ToString(const string &column_name) {
	return column_name + "." + child_filter->ToString(child_name);
}

bool StructFilter::Equals(const TableFilter &other_p) const {
	if (!TableFilter::Equals(other_p)) {
		return false;
	}
	auto &other = other_p.Cast<StructFilter>();
	return other.child_name == child_name && other.child_filter->Equals(*child_filter);
}

} // namespace duckdb
