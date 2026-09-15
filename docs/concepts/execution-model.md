# Execution Model

Running an LLM once per row is the naive way to execute a semantic query — and often the slowest and most
expensive one. iPDb's execution model for LLM-backed `PREDICT`/`LLM` calls exists to cut down how much actual
inference happens, without changing the result.

## Batching

Rather than issuing one API/model call per row, iPDb groups multiple rows into a single call and asks the model to
return a JSON array of per-row results, falling back to per-row calls only for rows whose batched result couldn't
be parsed or was otherwise invalid. This amortizes fixed per-call overhead (network round-trip, prompt boilerplate)
across many rows.

Aggregates over an `LLM` call (`AGG LLM ...`) work similarly, but combine rows toward a single summarized result
instead of one result per row.

Batch size and whether batching is used at all are configurable — see [Settings](../reference/config.md).

## Exact tuple deduplication

Real workloads often have rows that resolve to the exact same call — the same model, the same prompt template,
the same input column values. iPDb deduplicates these: within a single query execution, rows whose (model,
prompt, input) match exactly are grouped and sent to the LLM once, with that one result reused for every row in
the group instead of repeating the call. This is controlled by the `llm_use_cache` setting (`use_cache` as a
per-model option) — see [Settings](../reference/config.md).

Despite the setting's name, this isn't a cache that persists results across separate queries or sessions — it
only avoids redundant calls for exact-duplicate rows within the same execution.

## Semantic deduplication (clustering)

Beyond exact duplicates, many semantic predicates are applied to rows whose relevant input text is merely
*near*-duplicate or semantically equivalent (e.g. similarly-worded reviews, repeated boilerplate). The codebase
also has a mechanism for this: group rows into clusters by embedding-similarity of their prompt input columns,
send only one representative row per cluster to the LLM, propagate that result to the rest of the cluster, and
independently re-verify a sample of propagated results to catch bad propagation.

!!! note
    This clustering-based mechanism is currently disabled by default and isn't exposed as a session setting or
    documented build flag — treat exact tuple deduplication (above) as what actually runs today, and this section
    as describing the mechanism's intended design.

## Embeddings

`CREATE EMBEDDING` computes embeddings for a table/column once, up front, rather than recomputing them every time
a query needs them. Later semantic queries and clustering reuse these stored embeddings instead of re-embedding
the same rows. This is the mechanism behind "one-time embeddings" — you pay the embedding cost once at creation
time, not per query.

## Model auto-selection

A query can reference an `LLM` call without pinning it to one specific model configuration, in which case iPDb
picks one automatically. Selection strategies range from simple (first available, random) to a cost/quality-aware
strategy that scores candidate models against the workload before picking one. Which strategy is used, and how
candidates are scored, is configurable — see [Settings](../reference/config.md).

## Putting it together

For a query like:

```sql
SELECT p.name, r.review_text
FROM Product AS p
JOIN Review AS r ON p.product_id = r.product_id
WHERE LLM o4mini PROMPT 'is the {sentiment VARCHAR} of the {{review_text}} positive or negative' = 'negative';
```

the engine will, depending on configuration: push the join and any other filters so the LLM predicate only runs
over rows that need it, deduplicate rows with exactly matching `review_text` values before calling the model, and
batch the remaining calls together.
