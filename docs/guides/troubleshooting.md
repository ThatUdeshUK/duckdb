# Troubleshooting

## `PREDICT`, `CREATE MODEL`, or `LLM` fail with a syntax/catalog error

The binary you're running was likely built without the prediction extension enabled. Check:

```bash
./ipdb -c "PRAGMA platform;"
```

and confirm your build used `ENABLE_PREDICT=1`. Prebuilt release binaries and the Python package already have
this enabled; a from-source build needs it passed explicitly — see [Building iPDb](../development/build.md).

## A specific model type works but another doesn't (e.g. LLM works, tabular ONNX doesn't)

Each model backend (ONNX, llama.cpp, remote LLM API) is compiled in separately at build time via
`PREDICTOR_IMPL` and `ENABLE_LLM_API`. A build configured with `PREDICTOR_IMPL=llama_cpp`, for example, will not
have ONNX support even though the `CREATE TABULAR MODEL` syntax is always accepted by the parser. Rebuild with
the backend you need — see [Building iPDb](../development/build.md#build-options).

## Remote LLM calls fail with an authentication error

Check that iPDb can actually see your credentials:

- If you used `CREATE PERSISTENT SECRET ... SECRET <name>`, confirm the secret exists and is spelled the same way
  in your `CREATE LLM MODEL ... SECRET <name>` clause: `SELECT * FROM duckdb_secrets();`
- If you're relying on an environment variable (e.g. `OPENAI_API_KEY`) instead, confirm it's set in the shell
  that launched `ipdb`, and remember this approach only works for a single vendor at a time.
- Confirm the `API` base URL on the model matches what the vendor actually expects (including trailing `/v1/`
  where required).

See [Configuration](../getting-started/configuration.md) for the full setup.

## Queries are much slower than expected

This is almost always inference volume, not query planning. Before assuming something is broken:

- Run the query with `EXPLAIN`/`EXPLAIN ANALYZE` and check how many rows actually reach the `PREDICT`/`LLM`
  operator — a missing filter pushdown opportunity (e.g. a filter written in a way the optimizer can't push
  through the prediction) will run inference on far more rows than necessary.
- Check whether batching and exact tuple deduplication are enabled — see
  [Performance Tuning](../guides/performance-tuning.md). Disabling them (e.g. while debugging correctness) will
  make workloads with any redundant input data dramatically slower.
- For remote LLM APIs, check whether you're being rate-limited — the `req_per_min`-style option on
  `CREATE LLM MODEL` exists to stay under a vendor's rate limit, and setting it too low will serialize what could
  otherwise be concurrent calls.

## LLM output doesn't parse into the expected columns

The model's response has to match the structured format iPDb expects for the output columns declared in the
prompt. If you see parse failures or `NULL`s where you expected a value:

- Double check the output column type annotations in the prompt match the actual shape of what you're asking for
  (e.g. don't declare `INTEGER` for a field the model will naturally answer in prose).
- Try a more capable/instruction-following model — smaller local models are more likely to drift from the
  requested output format than larger remote ones.
- Batched calls fall back to per-row calls automatically when a batch response can't be parsed, but a systemically
  malformed prompt/output-type combination will still fail row-by-row.

## Getting more detail

Query profiling (`EXPLAIN ANALYZE`, or the benchmark runner's `--profile` flag) is the most direct way to see
where a query is actually spending time, including how many rows reach each `PREDICT`/`LLM` operator.

If none of the above explains what you're seeing, please open an issue with a minimal reproduction — see
[Contributing](../development/contributing.md).
