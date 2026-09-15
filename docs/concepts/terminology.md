# Terminology

A glossary of terms used throughout this documentation and in iPDb's SQL surface.

**Model**
: A catalog object created with `CREATE MODEL` (tabular/GNN) or `CREATE LLM MODEL` (local or remote LLM). Records
where the model lives, its input/output columns (or how to derive them from a prompt), and any backend options.
Persists with the database like a table or view.

**Model type**
: The kind of model a `CREATE MODEL`/`CREATE LLM MODEL` statement registers: **tabular** (classic feature-based
ML), **GNN** (graph neural network), **LM**/local language model, or **LLM** (chat/completion-style model, local
or remote). Also used for **embedding** models registered via `CREATE EMBEDDING`.

**`PREDICT`**
: The relational prediction operator. Usable as a table source (`FROM PREDICT(model, table)`) or as a scalar
expression (e.g. in a `WHERE` clause), producing one or more output columns per input row.

**`LLM ... PROMPT ...`**
: The LLM-flavored form of prediction: a natural-language prompt with input/output column placeholders, evaluated
against a specific LLM model. Can appear as a table source, a scalar/filter expression, a join predicate, or (with
`AGG`) an aggregate.

**Prompt input column**
: A column referenced inside a prompt's `{{column}}` placeholder, whose value is substituted into the prompt text
sent to the model for each row.

**Prompt output column**
: A column the model is asked to produce, declared inside the prompt with a type annotation (e.g.
`{summary VARCHAR}`), and returned as a typed column in the query result.

**Embedding**
: A vector representation of a column's text, computed once via `CREATE EMBEDDING` and reused by later semantic
queries and clustering rather than recomputed per query.

**Exact tuple deduplication**
: Sending only one LLM call for rows whose (model, prompt, input) match exactly, and reusing that result for the
rest — controlled by the `llm_use_cache` setting. Scoped to a single query execution, not a cross-query cache. See
[Execution Model](execution-model.md#exact-tuple-deduplication).

**Semantic clustering / deduplication**
: Grouping rows with near-identical (not necessarily exact) prompt input by embedding similarity so only one
representative per cluster is sent to the model, with the result propagated to the rest of the cluster. Present in
the codebase but currently disabled by default. See
[Execution Model](execution-model.md#semantic-deduplication-clustering).

**Batching**
: Sending multiple rows to the model in a single call (with a structured, per-row response format) instead of one
call per row, to amortize per-call overhead.

**Model auto-selection**
: Choosing which concrete model configuration serves an `LLM` call when the query doesn't pin one explicitly —
ranging from a simple first-available/random strategy to one that scores candidates on quality, cost, and latency.

**Secret**
: A DuckDB `CREATE SECRET`/`CREATE PERSISTENT SECRET` object holding an API credential (e.g. a bearer token),
referenced by name from a `CREATE LLM MODEL ... SECRET <name>` clause instead of embedding the key in SQL text or
relying on an environment variable.

**Semantic query**
: A query that uses `LLM`/`PREDICT` over natural-language or otherwise unstructured data to answer something that
isn't expressible with plain relational predicates — e.g. "which reviews are negative," "which motherboards are
compatible with this CPU."
