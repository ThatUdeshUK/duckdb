# SQL Reference

This page is the syntax reference for every SQL construct iPDb adds on top of DuckDB. Everything else in DuckDB's
SQL dialect is unchanged — see [DuckDB's own SQL reference](https://duckdb.org/docs/stable/sql/introduction) for
that.

!!! warning "Availability depends on your build"
    All of this syntax parses regardless of build configuration, but a statement only *runs* if the matching
    backend was compiled in. `CREATE TABULAR MODEL` needs `PREDICTOR_IMPL=onnx`; a local (`.gguf`) `CREATE LLM
    MODEL` needs `PREDICTOR_IMPL=llama_cpp`; a remote `CREATE LLM MODEL`/`CREATE EMBED MODEL` needs
    `ENABLE_LLM_API=1`. See [Building iPDb](../development/build.md#build-options).

## `CREATE MODEL` (tabular / GNN)

```sql
CREATE [OR REPLACE] [TEMP] {TABULAR | LM | GNN} MODEL [IF NOT EXISTS] model_name
  PATH '<path>'
  <model_on>
```

`<model_on>` is one of:

```sql
-- bound to a table: PREDICT(model, table) requires no extra column matching
ON TABLE table_name
  [FEATURES (col1, col2, ...) | FEATURES * EXCLUDE (col1, col2, ...)]
  OUTPUT (col1 TYPE1, col2 TYPE2, ...)
  [OPTIONS {key: value, ...}]

-- not bound to a table: matched against whatever table PREDICT(model, table) is called with
ON FEATURES (col1, col2, ...) OUTPUT (col1 TYPE1, ...) [OPTIONS {...}]
ON FEATURES * EXCLUDE (col1, ...) OUTPUT (col1 TYPE1, ...) [OPTIONS {...}]

-- GNN: separate node and edge tables
ON NODES nodes_table [FEATURES (...) | FEATURES * EXCLUDE (...)]
   EDGES edges_table [FEATURES (...) | FEATURES * EXCLUDE (...)]
   OUTPUT (col1 TYPE1, ...)
   [OPTIONS {...}]
```

- `TABULAR` / `LM` / `GNN` select the model type; the matching backend must be compiled in.
- `FEATURES (a, b, c)` names the exact input columns; `FEATURES * EXCLUDE (x, y)` uses every column except the
  ones listed.
- `OUTPUT (...)` declares the result columns and their types — any DuckDB type is valid; column constraints are
  not.
- `OPTIONS {...}` accepts backend-tuning keys — see [Settings](config.md#per-model-options) for what each backend
  reads.

Example:

```sql
CREATE TABULAR MODEL iris_cls PATH 'iris_cls.onnx'
  ON TABLE iris
  OUTPUT (class INTEGER);

SELECT * FROM PREDICT(iris_cls, iris) AS p WHERE p.class = 2;
```

## `CREATE LLM MODEL`

```sql
CREATE [OR REPLACE] [TEMP] LLM MODEL [IF NOT EXISTS] model_name
  PATH '<local_gguf_path_or_remote_model_id>'
  ON PROMPT
  [API 'https://...']
  [SECRET secret_name]
  [OPTIONS {key: value, ...}]
```

- `PATH` decides the backend at query time, not at `CREATE` time: a path containing `.gguf` is loaded locally via
  llama.cpp; anything else is treated as a remote model identifier (e.g. `'gpt-4o-mini'`, `'o4-mini'`) sent to
  `API`.
- `API '<url>'` is the OpenAI-compatible base URL for a remote model. If omitted, iPDb falls back to the
  `OPENAI_API_BASE` environment variable, then an empty base URL.
- `SECRET secret_name` must reference an existing secret (checked when the model is created); its `bearer_token`
  is sent as `Authorization: Bearer <token>` on every call. Omit it for a local `.gguf` model, or when relying on
  an environment-variable API key instead — see [Configuration](../getting-started/configuration.md).
- `ON PROMPT` is mandatory — there's no `ON TABLE` form for `CREATE LLM MODEL`; the input/output columns are
  inferred per-call from the prompt itself (see below), not fixed at creation time.

Examples:

```sql
CREATE LLM MODEL o4mini PATH 'o4-mini' ON PROMPT
  API 'https://api.openai.com/v1/' SECRET openai_key
  OPTIONS {"req_per_min": 30};

CREATE LLM MODEL gemma2 PATH '/path/to/gemma-2-2b-it.gguf' ON PROMPT;
```

## `CREATE EMBED MODEL` and `CREATE EMBEDDING`

These are two different statements — don't confuse them.

**`CREATE EMBED MODEL`** registers an embedding-generating model (same shape as `CREATE LLM MODEL`, minus
`ON PROMPT`):

```sql
CREATE [OR REPLACE] [TEMP] EMBED MODEL [IF NOT EXISTS] model_name
  PATH '<local_path_or_remote_model_id>'
  [API 'https://...']
  [SECRET secret_name]
  [OPTIONS {...}]
```

**`CREATE EMBEDDING`** attaches a registered embed model to a table column, so its embeddings are computed once
and reused later:

```sql
CREATE EMBEDDING [IF NOT EXISTS] embedding_name
  ON table_name (column_name)
  USING embed_model_name
  [SIZE n]
```

`SIZE` is optional (defaults to unspecified/dynamic). See [Execution Model](../concepts/execution-model.md#embeddings)
for why this two-step (model, then embedding) shape exists.

```sql
CREATE EMBED MODEL text_embed PATH 'text-embedding-3-small' API 'https://api.openai.com/v1/' SECRET openai_key;
CREATE EMBEDDING review_embed ON reviews (review_text) USING text_embed;
```

There's also an inline form usable directly inside a query, without a stored `CREATE EMBEDDING`:

```sql
EMBED model_name(COLUMN column_name, table_ref)
```

which returns one column, `vec`, of type `ARRAY(FLOAT, 384)`.

## `PREDICT` (tabular / GNN)

```sql
-- as a table source
TABULAR model_name(table_ref)
PREDICT(model_name, table_ref)
GNN(model_name, nodes_table_ref, edges_table_ref)

-- with an alias, like any FROM-item
FROM PREDICT(model_name, table_ref) AS alias
```

`table_ref` can be a table name, a subquery, or a join result — anything that produces rows. Output columns come
from the model's `OUTPUT (...)` declaration.

## `LLM ... PROMPT ...`

The prompt-driven form. The model name is always optional — omit it to let iPDb
[auto-select a model](../concepts/execution-model.md#model-auto-selection).

```sql
-- as a table source, reading from an explicit input relation
LLM [model_name] (PROMPT '<prompt text>' ON table_ref)

-- as a scalar expression (filter, join predicate, GROUP BY key, SELECT list),
-- reading whatever columns are already in scope
LLM model_name PROMPT '<prompt text>'
LLM '<prompt text>'                      -- no model name: auto-select

-- as an aggregate (note the keyword order: AGG before LLM)
AGG LLM [model_name] (PROMPT '<prompt text>')
```

### Prompt placeholders

- **Input columns**: `` {{column}} `` or `` {{table.column}} `` — substituted with that row's value before the
  prompt is sent to the model.
- **Output columns**: `` {column_name TYPE} `` (space-separated — not a colon) where `TYPE` is one of `INTEGER`,
  `VARCHAR`, `BOOLEAN` (or `BOOL`), `DOUBLE`. The model's response is parsed back into a typed column with this
  name.
- A scalar `LLM ... PROMPT ...` expression allows **at most one** output placeholder. A table-source `LLM(...)`
  call may declare several, producing multiple output columns.

```sql
-- table source: two output columns, one input column
SELECT title, genre, main_character
FROM LLM o4mini (PROMPT 'extract the {genre VARCHAR} and {main_character VARCHAR} from the {{plot}}' ON movies);

-- scalar filter: one output column, used as a boolean predicate
SELECT p.name, r.review_text
FROM Product AS p JOIN Review AS r ON p.product_id = r.product_id
WHERE LLM o4mini PROMPT 'is the {sentiment VARCHAR} of the {{review_text}} positive or negative' = 'negative';

-- scalar join predicate
SELECT p.name
FROM Product AS p
JOIN Product AS o
  ON LLM o4mini PROMPT 'is CPU {{o.name}} {compatible BOOLEAN} with motherboard {{p.name}}';

-- aggregate, one row of output per group
SELECT category_id, AGG LLM o4mini (PROMPT 'what is the computer {component VARCHAR} from the {{description}}')
FROM products GROUP BY category_id;

-- table source with no input relation at all ("table generation")
SELECT state, state_tax
FROM LLM o4mini PROMPT 'get all the {state VARCHAR} and their {state_tax DOUBLE} pairs in the US';
```

## Introspecting models and embeddings

```sql
SELECT * FROM ipdb_models();
SELECT * FROM ipdb_embeddings();
SELECT * FROM duckdb_secrets();
```

See [CLI Reference](cli.md#table-functions) for their columns.
