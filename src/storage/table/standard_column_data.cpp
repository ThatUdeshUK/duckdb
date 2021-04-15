#include "duckdb/storage/table/standard_column_data.hpp"
#include "duckdb/storage/table/scan_state.hpp"
#include "duckdb/storage/table/update_segment.hpp"
#include "duckdb/storage/table/append_state.hpp"
#include "duckdb/storage/table/persistent_segment.hpp"
#include "duckdb/storage/table/transient_segment.hpp"

namespace duckdb {

StandardColumnData::StandardColumnData(DatabaseInstance &db, DataTableInfo &table_info, LogicalType type,
                                       idx_t column_idx, ColumnData *parent)
    : ColumnData(db, table_info, move(type), column_idx, parent), validity(db, table_info, column_idx, this) {
}

bool StandardColumnData::CheckZonemap(ColumnScanState &state, TableFilter &filter) {
	if (!state.segment_checked) {
		state.segment_checked = true;
		if (!state.current) {
			return true;
		}
		if (state.current->stats.CheckZonemap(filter)) {
			return true;
		}
		return false;
		// if (state.updates) {
		// 	return state.updates->GetStatistics().CheckZonemap(filter);
		// } else {
		// 	return false;
		// }
	} else {
		return true;
	}
}

void StandardColumnData::InitializeScan(ColumnScanState &state) {
	// initialize the current segment
	state.current = (ColumnSegment *)data.GetRootSegment();
	state.row_index = 0;
	state.initialized = false;

	// initialize the validity segment
	ColumnScanState child_state;
	validity.InitializeScan(child_state);
	state.child_states.push_back(move(child_state));
}

void StandardColumnData::InitializeScanWithOffset(ColumnScanState &state, idx_t row_idx) {
	state.current = (ColumnSegment *)data.GetSegment(row_idx);
	state.row_index = row_idx;
	state.initialized = false;

	// initialize the validity segment
	ColumnScanState child_state;
	validity.InitializeScanWithOffset(child_state, row_idx);
	state.child_states.push_back(move(child_state));
}

void StandardColumnData::Scan(Transaction &transaction, ColumnScanState &state, Vector &result) {
	if (!state.initialized) {
		state.current->InitializeScan(state);
		state.initialized = true;
	}
	// fetch validity data
	validity.Scan(transaction, state.child_states[0], result);

	// perform a scan of this segment
	ScanVector(transaction, state, result);

	// move over to the next vector
	state.Next();
}

void StandardColumnData::IndexScan(ColumnScanState &state, Vector &result, bool allow_pending_updates) {
	if (!state.initialized) {
		state.current->InitializeScan(state);
		state.initialized = true;
	}
	// // perform a scan of this segment
	validity.IndexScan(state.child_states[0], result, allow_pending_updates);

	if (!allow_pending_updates && state.current->updates && state.current->updates->HasUncommittedUpdates(state.row_index)) {
		throw TransactionException("Cannot create index with outstanding updates");
	}
	ScanCommitted(state, result);

	// move over to the next vector
	state.Next();
}

void StandardColumnData::InitializeAppend(ColumnAppendState &state) {
	ColumnData::InitializeAppend(state);

	ColumnAppendState child_append;
	validity.InitializeAppend(child_append);
	state.child_appends.push_back(move(child_append));
}

void StandardColumnData::AppendData(ColumnAppendState &state, VectorData &vdata, idx_t count) {
	ColumnData::AppendData(state, vdata, count);

	validity.AppendData(state.child_appends[0], vdata, count);
}

void StandardColumnData::RevertAppend(row_t start_row) {
	ColumnData::RevertAppend(start_row);

	validity.RevertAppend(start_row);
}

void StandardColumnData::Update(Transaction &transaction, Vector &update_vector, Vector &row_ids, idx_t count, bool is_root) {
	validity.Update(transaction, update_vector, row_ids, count);
	ColumnData::Update(transaction, update_vector, row_ids, count, is_root);
}

void StandardColumnData::Fetch(ColumnScanState &state, row_t row_id, Vector &result) {
	// fetch validity mask
	if (state.child_states.empty()) {
		ColumnScanState child_state;
		state.child_states.push_back(move(child_state));
	}
	validity.Fetch(state.child_states[0], row_id, result);
	ColumnData::Fetch(state, row_id, result);
}

void StandardColumnData::FetchRow(ColumnFetchState &state, Transaction &transaction, row_t row_id, Vector &result,
                                  idx_t result_idx) {
	// find the segment the row belongs to
	if (state.child_states.empty()) {
		auto child_state = make_unique<ColumnFetchState>();
		state.child_states.push_back(move(child_state));
	}
	validity.FetchRow(*state.child_states[0], transaction, row_id, result, result_idx);
	ColumnData::FetchRow(state, transaction, row_id, result, result_idx);
}

unique_ptr<BaseStatistics> StandardColumnData::GetStatistics() {
	auto base_stats = ColumnData::GetStatistics();
	base_stats->validity_stats = validity.GetStatistics();
	return base_stats;
}

void StandardColumnData::CommitDropColumn() {
	ColumnData::CommitDropColumn();
	validity.CommitDropColumn();
}

void StandardColumnData::Checkpoint(TableDataWriter &writer) {
	ColumnData::Checkpoint(writer);
	validity.Checkpoint(writer);
}

void StandardColumnData::Initialize(PersistentColumnData &column_data) {
	auto &persistent = (StandardPersistentColumnData &)column_data;
	ColumnData::Initialize(column_data);
	validity.Initialize(*persistent.validity);
}

unique_ptr<PersistentColumnData> StandardColumnData::Deserialize(DatabaseInstance &db, Deserializer &source,
                                                                 const LogicalType &type) {
	auto result = make_unique<StandardPersistentColumnData>();
	BaseDeserialize(db, source, type, *result);
	result->validity = ValidityColumnData::Deserialize(db, source);
	return move(result);
}

} // namespace duckdb
