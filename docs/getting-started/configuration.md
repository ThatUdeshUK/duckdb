# Configuration

## API credentials for remote LLMs

Remote LLM/embedding models (anything created with `API '<url>'`) need a credential. iPDb supports two ways to
provide one:

### Secrets (recommended)

```sql
CREATE PERSISTENT SECRET openai_key (TYPE http, bearer_token '<openai_api_key>');
CREATE PERSISTENT SECRET google_key (TYPE http, bearer_token '<google_api_key>');
```

Reference a secret by name from the model:

```sql
CREATE LLM MODEL o4mini PATH 'o4-mini' ON PROMPT
  API 'https://api.openai.com/v1/' SECRET openai_key;
```

The secret's `bearer_token` is sent as `Authorization: Bearer <token>` on every call to that model. This is the
only mechanism that supports multiple vendors/keys at once — each model picks its own secret. `PERSISTENT` secrets
survive across sessions (stored in DuckDB's secret storage); drop it for an in-memory-only secret scoped to the
current session.

Check what's configured:

```sql
SELECT * FROM duckdb_secrets();
```

### Environment variable

```bash
export OPENAI_API_KEY="<api_key>"
```

Add this to your shell profile (e.g. `.bashrc`) for it to persist across sessions. This is simpler for a
single-vendor setup, but **limits you to one vendor** — you can't mix an OpenAI-backed model and a
differently-keyed model this way. Prefer secrets once you have more than one remote model.

If a model's `CREATE LLM MODEL` omits `API`, iPDb falls back to the `OPENAI_API_BASE` environment variable for the
base URL (and then to an empty base URL if that isn't set either) — set it if you're pointing at a non-default
OpenAI-compatible endpoint without specifying `API` on every model.

## Local model paths

- **Local LLM** (`PREDICTOR_IMPL=llama_cpp`): `PATH` on `CREATE LLM MODEL` points at a `.gguf` file on disk. No
  `API`/`SECRET` needed.
- **Tabular/GNN** (`PREDICTOR_IMPL=onnx`): `PATH` on `CREATE TABULAR MODEL`/`CREATE GNN MODEL` points at a `.onnx`
  file on disk.

Whichever of these you need must be compiled into your build — see
[Building iPDb](../development/build.md#build-options) for `PREDICTOR_IMPL`/`ENABLE_LLM_API`.

## Session settings

Everything that controls batching, exact tuple deduplication, filter pushdown, and model auto-selection is a normal
`SET` option, either session-wide or overridden per model via `OPTIONS {...}` on the `CREATE ... MODEL` statement.
See the full list in [Settings](../reference/config.md); the ones you'll touch most while getting a workload
running smoothly are covered in [Performance Tuning](../guides/performance-tuning.md).

```sql
SET llm_use_batch = true;
SET ml_batch_size = 16;
SET llm_use_cache = true;
```

## Next steps

Head to [Quickstart](quickstart.md) if you haven't registered your first model yet, or
[Working with Input Data](../guides/input-data.md) to get your data into a shape `PREDICT`/`LLM` can consume.
