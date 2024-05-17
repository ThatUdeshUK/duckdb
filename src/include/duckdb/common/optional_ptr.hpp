//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/optional_ptr.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/exception.hpp"
#include "duckdb/common/unique_ptr.hpp"
#include "duckdb/common/shared_ptr.hpp"

namespace duckdb {

template <class T>
class optional_ptr { // NOLINT: mimic std casing
public:
	optional_ptr() : ptr(nullptr) {
	}
	optional_ptr(T *ptr_p) : ptr(ptr_p) { // NOLINT: allow implicit creation from pointer
	}
	optional_ptr(T &ref) : ptr(&ref) { // NOLINT: allow implicit creation from reference
	}
	optional_ptr(const unique_ptr<T> &ptr_p) : ptr(ptr_p.get()) { // NOLINT: allow implicit creation from unique pointer
	}
	optional_ptr(const shared_ptr<T> &ptr_p) : ptr(ptr_p.get()) { // NOLINT: allow implicit creation from shared pointer
	}

	void CheckValid() const {
		if (!ptr) {
			throw InternalException("Attempting to dereference an optional pointer that is not set");
		}
	}

	operator bool() const { // NOLINT: allow implicit conversion to bool
		return ptr;
	}
	T &operator*() {
		CheckValid();
		return *ptr;
	}
	const T &operator*() const {
		CheckValid();
		return *ptr;
	}
	T *operator->() {
		CheckValid();
		return ptr;
	}
	const T *operator->() const {
		CheckValid();
		return ptr;
	}
	T *get() { // NOLINT: mimic std casing
		// CheckValid();
		return ptr;
	}
	const T *get() const { // NOLINT: mimic std casing
		// CheckValid();
		return ptr;
	}
	// this looks dirty - but this is the default behavior of raw pointers
	T *get_mutable() const { // NOLINT: mimic std casing
		// CheckValid();
		return ptr;
	}

	bool operator==(const optional_ptr<T> &rhs) const {
		return ptr == rhs.ptr;
	}

	bool operator!=(const optional_ptr<T> &rhs) const {
		return ptr != rhs.ptr;
	}

private:
	T *ptr;
};

} // namespace duckdb
