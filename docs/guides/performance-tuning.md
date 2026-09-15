# Performance Tuning

For LLM-backed queries in particular, wall-clock time is dominated by how many inference calls actually happen,
not by ordinary relational overhead. Most tuning here is about controlling that — see
[Execution Model](../concepts/execution-model.md) for the mechanisms these settings toggle.

## Session settings

Set with `SET <name> = <value>;` in the shell or over any client connection. Full list and defaults:
[Settings Reference](../reference/config.md).

| Setting | Purpose |
|---|---|
| `pull_predict_filter` | Controls whether filters are pulled up/pushed around `PREDICT`/`LLM` operators. Disable only when debugging a suspected optimizer issue — leaving it on is almost always faster. |
| `llm_use_batch` | Controls whether multiple rows are batched into a single LLM call instead of one call per row. |
| `ml_batch_size` | Number of rows grouped per batched inference call. |
| `llm_use_cache` | Controls exact tuple deduplication — rows with an exactly matching (model, prompt, input) share a single LLM call instead of each triggering one. Not a cross-query result cache, despite the name. |

Example, from `demo.sql`:

```sql
SET llm_use_batch = false;
SET ml_batch_size = 16;
SET llm_use_cache = false;
SET pull_predict_filter = false;
```

Turning these off is useful for isolating a correctness question (e.g. "is batching changing my results?"), but
for normal use, leave batching/dedup/filter-pushdown on and tune `ml_batch_size` instead of disabling batching
outright. Disabling `llm_use_cache` in particular means every exact-duplicate row pays for its own inference call
— useful only when you specifically need per-row calls (e.g. non-deterministic model output on purpose).

## Rate limiting remote LLM calls

`CREATE LLM MODEL ... OPTIONS {...}` accepts a `req_per_min` key to cap how fast iPDb calls a given remote model,
which keeps you under a vendor's rate limit:

```sql
CREATE LLM MODEL o4mini PATH 'o4-mini' ON PROMPT API 'https://api.openai.com/v1/' SECRET openai_key
  OPTIONS {"req_per_min": 30};
```

Set this as high as your API tier actually allows — an unnecessarily low value serializes calls that could
otherwise run concurrently and directly slows down every query against that model.

## Reduce inference volume at the query level

Before tuning settings, check whether the query itself is causing more inference than necessary:

- Filter as early/aggressively as possible on non-`PREDICT`/`LLM` columns first — the optimizer will push
  compatible filters through the prediction automatically (see [Architecture](../concepts/architecture.md)), but
  writing filters in a pushable form (plain column comparisons, not expressions that obscure the dependency) helps
  it do so.
- If you only need the first N matching rows, make sure the `LIMIT` is on the outermost query rather than buried
  under an aggregation or join that would force full evaluation anyway.
- Prefer reusing an existing `CREATE EMBEDDING` over recomputing embeddings ad hoc inside a query.

## Choosing a model

- Smaller/local models (llama.cpp) avoid network latency and per-call cost, at some risk to output quality and
  format-following (see [Troubleshooting](troubleshooting.md#llm-output-doesnt-parse-into-the-expected-columns)).
- If you let iPDb auto-select a model instead of pinning one, review `model_select_strategy` and the related
  `model_select_*` settings in the [Settings Reference](../reference/config.md) — the cost/quality-aware strategy
  is usually a better default than picking one at random once you have more than one candidate model configured.

## Measuring

Use `EXPLAIN ANALYZE`, or the benchmark runner's `--profile` flag, to see how many rows actually reach each
`PREDICT`/`LLM` operator and where time is going before and after a tuning change.
