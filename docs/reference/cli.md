# CLI Reference

iPDb's shell binary is `ipdb` — the standard DuckDB CLI (`tools/shell/`), unmodified, built with the model/LLM
extension available whenever the binary was compiled with it enabled. There are no iPDb-specific flags or REPL
behavior beyond the SQL statements themselves; everything below is stock DuckDB CLI usage.

## Invocation

```bash
ipdb                      # in-memory database
ipdb mydata.duckdb         # open/create a persistent database file
ipdb -c "SELECT 1"         # run one statement and exit
ipdb mydata.duckdb < script.sql   # run a script non-interactively
```

Common flags (see `ipdb -help` for the full, version-matched list):

| Flag | Effect |
|---|---|
| `-c "<sql>"` | Run a single statement/command and exit. |
| `-json` | Output results as JSON. |
| `-csv` | Output results as CSV. |
| `-readonly` | Open the database read-only. |
| `-init <file>` | Run a file of dot-commands/SQL before the prompt/script starts. |
| `-no-stdin` | Don't read commands from stdin. |

## Dot-commands

The shell's dot-commands (`.tables`, `.schema`, `.mode`, `.output`, `.read`, `.help`, ...) are unchanged from
DuckDB — see [Output Formats](output-formats.md) for the ones most relevant to exporting `PREDICT`/`LLM` results,
or `.help` in the shell for the complete list.

## Table functions

These are the main entry points for inspecting what's registered in the current database.

### `ipdb_models()`

One row per registered model (`CREATE MODEL`/`CREATE TABULAR MODEL`/`CREATE LLM MODEL`/`CREATE EMBED MODEL`).

| Column | Type | Description |
|---|---|---|
| `database_name` | `VARCHAR` | Database the model belongs to. |
| `database_oid` | `BIGINT` | Database OID. |
| `schema_name` | `VARCHAR` | Schema the model belongs to. |
| `schema_oid` | `BIGINT` | Schema OID. |
| `model_name` | `VARCHAR` | Model name. |
| `model_oid` | `BIGINT` | Model OID. |
| `comment` | `VARCHAR` | Comment, if any. |
| `tags` | `MAP(VARCHAR, VARCHAR)` | Tags, if any. |
| `temporary` | `BOOLEAN` | Whether the model is temporary. |
| `model_type` | `VARCHAR` | `TABULAR` / `LM` / `GNN` / `LLM` / `EMBED`. |
| `model_path` | `VARCHAR` | The `PATH` the model was created with. |
| `rel_name` | `VARCHAR` | Bound table name, for `ON TABLE`-style models (empty otherwise). |
| `input_set_count` | `BIGINT` | Number of explicit `FEATURES (...)` columns. |
| `exclude_set_count` | `BIGINT` | Number of `FEATURES * EXCLUDE (...)` columns. |
| `opt_rel_name` | `VARCHAR` | Second (edges) table name, for GNN models. |
| `opt_set_count` | `BIGINT` | Feature column count for the GNN edges table. |
| `exclude_opt_set_count` | `BIGINT` | Excluded feature column count for the GNN edges table. |
| `output_count` | `BIGINT` | Number of declared `OUTPUT (...)` columns. |
| `options` | `MAP(VARCHAR, VARCHAR)` | The model's `OPTIONS {...}`. |
| `sql` | `VARCHAR` | The original `CREATE ... MODEL` statement. |

### `ipdb_embeddings()`

One row per `CREATE EMBEDDING`.

| Column | Type | Description |
|---|---|---|
| `database_name` | `VARCHAR` | Database the embedding belongs to. |
| `database_oid` | `BIGINT` | Database OID. |
| `schema_name` | `VARCHAR` | Schema the embedding belongs to. |
| `schema_oid` | `BIGINT` | Schema OID. |
| `embedding_name` | `VARCHAR` | Embedding name. |
| `embedding_oid` | `BIGINT` | Embedding OID. |
| `comment` | `VARCHAR` | Comment, if any. |
| `tags` | `MAP(VARCHAR, VARCHAR)` | Tags, if any. |
| `temporary` | `BOOLEAN` | Whether the embedding is temporary. |
| `table_name` | `VARCHAR` | Table the embedding is attached to. |
| `column_name` | `VARCHAR` | Column the embedding covers. |
| `model_catalog` | `VARCHAR` | Catalog of the backing `EMBED` model. |
| `model_schema` | `VARCHAR` | Schema of the backing `EMBED` model. |
| `model_name` | `VARCHAR` | Name of the backing `EMBED` model. |
| `embedding_size` | `BIGINT` | The `SIZE` given at creation (`0` if unspecified). |
| `sql` | `VARCHAR` | The original `CREATE EMBEDDING` statement. |

### `duckdb_secrets()`

Standard DuckDB secrets introspection — lists configured secrets (name, type, whether persisted, etc.), including
any used by `CREATE LLM MODEL ... SECRET <name>`. See
[DuckDB's secrets documentation](https://duckdb.org/docs/stable/configuration/secrets_manager) for its columns.

## See also

- [SQL Reference](sql.md) for `CREATE MODEL`/`CREATE LLM MODEL`/`CREATE EMBEDDING`/`PREDICT`/`LLM` syntax.
- [Settings](config.md) for every `SET` option.
- [Output Formats](output-formats.md) for shell output modes and exporting results.
