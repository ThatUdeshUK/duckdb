# Testing

iPDb uses DuckDB's standard test infrastructure unchanged: sqllogictest files (`test/sql/**/*.test`) for SQL-level
behavior, plus C++ unit tests under `test/` for anything that needs to exercise internals directly (concurrent
connections, exotic edge cases, etc.).

## Running tests

Build a debug test binary and run it:

```bash
make unittest
```

This builds `build/debug/test/unittest` and runs it.

Run a single sqllogictest file:

```bash
build/debug/test/unittest test/sql/path/to/file.test
```

Run every test tagged with a given tag:

```bash
build/debug/test/unittest "[tag]"
```

Standard fast/full split still applies:

- `make unit` — the fast unit test suite (~1 minute).
- `make allunit` — the full suite (~1 hour), including slow tests (named `*.test_slow` in sqllogictest, or tagged
  `[.]` in C++ tests).

## Coverage of PREDICT / LLM features

There is currently **no dedicated `test/sql/**` sqllogictest coverage** for `PREDICT`, `CREATE MODEL`, `CREATE LLM
MODEL`, or `CREATE EMBEDDING`. Those features are exercised manually today via `demo.sql` at the repo root, and via
the `benchmark/semantic_benchmark` and `benchmark/sem_bench_movie` `.benchmark` suites — not automated tests.

**If you're adding new PREDICT/LLM functionality, prefer adding real sqllogictest coverage under `test/sql/`**
rather than assuming parity with the existing test suite. This is an active gap, not the intended long-term state.

## Writing tests

Follow upstream DuckDB testing conventions:

- Prefer sqllogictest (`.test`) over C++ tests unless you specifically need to test concurrent connections or
  other behavior sqllogictest can't express.
- Cover multiple types, especially numerics, strings, and complex/nested types.
- Test invalid/unexpected usage, not just the happy path — try to trigger the exceptions your code raises.
- See DuckDB's own [testing documentation](https://duckdb.org/dev/testing) for the sqllogictest format in detail.

## Formatting

Run the formatter before sending a PR:

```bash
make format-fix   # apply formatting
make format-check # check without modifying files
```

`clang-format` `11.0.1` is required for consistent results:

```bash
python3 -m pip install clang-format==11.0.1
```
