# Building iPDb

iPDb builds on top of DuckDB's standard `make`-based build system, extended with a handful of extra variables
that turn on the prediction/LLM extension. A plain `make` from upstream DuckDB will **not** give you
`PREDICT`/`CREATE MODEL` support — you need to opt in explicitly (see below).

## Prerequisites

### ONNX runtime (optional — tabular/GNN models)

Needed only if you plan to run pre-trained `.onnx` models. Skip this if you're only doing LLM inference.

- **Build from scratch**: follow the [ONNX Runtime build guide](https://onnxruntime.ai/docs/build/inferencing.html).
- **Use a release binary**: download a build from the
  [onnxruntime releases](https://github.com/microsoft/onnxruntime/releases/tag/v1.19.2) page. Version `1.19.2` is
  the version verified to work on `linux-amd64`.

Either way, point iPDb at the install location:

```bash
export ONNX_INSTALL_PREFIX=<onnx_runtime_installed_path>
```

### llama.cpp (optional — local LLMs)

Needed only if you plan to run local LLMs in GGUF format. Skip this if you're only using remote LLM APIs or ONNX
models.

```bash
git clone https://github.com/ggml-org/llama.cpp
cd llama.cpp
cmake -B build
cmake --build build --config Release -j 8
```

`Release` builds are strongly preferred for inference performance. See the
[llama.cpp build guide](https://github.com/ggml-org/llama.cpp/blob/master/docs/build.md) for GPU backends, debug
builds, and troubleshooting.

```bash
export LIBLLAMA_INSTALL_PREFIX="<llama_cpp_repo>/build"
```

## Build

```bash
make debug GEN=ninja -j12 CORE_EXTENSIONS='httpfs' ENABLE_PREDICT=1 PREDICTOR_IMPL=llama_cpp ENABLE_LLM_API=True DISABLE_SANITIZER=1
```

This produces a `ipdb` CLI binary under `build/debug/` (or `build/release/` for a release build).

### Build options

| Variable | Values | Meaning |
|---|---|---|
| `GEN` | `ninja` | Optional. Use Ninja instead of Make for build parallelization (pairs with `-jN`). |
| `ENABLE_PREDICT` | `1` | Enables the `PREDICT`/`CREATE MODEL` machinery (`-DENABLE_PREDICT`). Required for any prediction feature. |
| `PREDICTOR_IMPL` | `onnx` \| `llama_cpp` | Selects the native model backend compiled into `third_party/predictors/`. `onnx` runs pre-trained `.onnx` models (requires `ONNX_INSTALL_PREFIX`); `llama_cpp` runs local LLMs in GGUF format (requires `LIBLLAMA_INSTALL_PREFIX`). `torchscript` is deprecated and unsupported. |
| `ENABLE_LLM_API` | `1` / `True` | Enables calling remote LLMs over HTTP (OpenAI-compatible APIs). Automatically pulls in the `httpfs` core extension. |
| `DISABLE_LLM_API` | `1` | The top-level `Makefile` forces `ENABLE_PREDICT=1 -DENABLE_LLM_API=1` by default — pass this if you need a build **without** LLM API support. |
| `CORE_EXTENSIONS` | e.g. `'httpfs'` | Standard DuckDB variable for bundling core extensions into the build. |
| `DISABLE_SANITIZER` | `1` | Standard DuckDB variable; speeds up debug builds by skipping ASan/UBSan instrumentation. |

Standard DuckDB build targets apply on top of these: `make debug`, `make release`, `make unit` (fast unit tests),
`make allunit` (full suite, ~1 hour), `make format-fix` / `make format-check`.

!!! tip "Runaway parallel builds"
    If a high `-j` value locks up your machine (common on memory-constrained hosts), cap it:
    `CMAKE_BUILD_PARALLEL_LEVEL=4 GEN=ninja make`.

## Building with Docker

```bash
git clone https://github.com/purduedb/iPDb
cd iPDb
docker build -t ipdb .
```

The bundled `Dockerfile` currently builds ONNX (tabular/GNN) support only; build from source for LLM inference.
See [Installation](../getting-started/installation.md#with-docker) for how to run the resulting image.

## Verifying the build

```bash
./build/debug/ipdb -c "PRAGMA platform;"
```

## Where things live

If you're navigating the source rather than just building it, see [Architecture](../concepts/architecture.md) for
an overview of how the prediction/LLM extension is organized across the DuckDB pipeline (parser, planner,
optimizer, execution, backends).
