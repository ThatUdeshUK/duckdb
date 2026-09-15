# Installation

iPDb can be installed as a prebuilt CLI binary, as a Python package, or built from source. Pick whichever fits
your workflow — all three expose the same SQL surface.

## Prebuilt CLI binary (Linux)

Prebuilt Linux binaries for `amd64` and `arm64` are published on [Releases](https://github.com/purduedb/iPDb/releases).
These builds have LLM API support enabled (`ENABLE_PREDICT=1 ENABLE_LLM_API=1`) but do **not** include the native
ONNX or llama.cpp backends — use them if you only need remote LLM inference.

```bash
curl -LO https://github.com/purduedb/iPDb/releases/download/v1.1.0/ipdb-cli-linux-amd64.zip
unzip ipdb-cli-linux-amd64.zip
chmod +x ipdb
./ipdb
```

Swap `amd64` for `arm64` if you're on an ARM64 host, and pick the latest release tag from the
[Releases page](https://github.com/purduedb/iPDb/releases).

## Python package

A `duckdb`-compatible Python package (`ipdb`) is published for each release, built with iPDb as the underlying
engine. The API is identical to the
[DuckDB Python client](https://duckdb.org/docs/stable/clients/python/overview), plus the semantic operators
described in this documentation.

```bash
pip install ipdb-<latest_version>.tar.gz
```

Download the `.tar.gz` for the version you want from [Releases](https://github.com/purduedb/iPDb/releases); `pip`
builds the wheel and installs it locally.

## Building from source

Building from source gives you the most control — in particular, it's the only way to enable the native ONNX or
llama.cpp backends. See [Building iPDb](../development/build.md) for full instructions, including the optional
ONNX/llama.cpp prerequisites and all `make` options.

Quick version, for LLM-only usage (no local model backends):

```bash
make debug GEN=ninja -j12 CORE_EXTENSIONS='httpfs' ENABLE_PREDICT=1 ENABLE_LLM_API=True DISABLE_SANITIZER=1
```

This produces a `ipdb` binary shell under `build/debug/`.

## With Docker

> The published Dockerfile currently builds ONNX (tabular/GNN) model support only. Build from source if you need
> LLM inference.

```bash
git clone https://github.com/purduedb/iPDb
cd iPDb
docker build -t ipdb .
docker run -it -v <path_to_data_dir>:/data --name=ipdb_container ipdb /bin/bash
```

Drop the `-v` mount if you just want to try iPDb without persisting a models/data directory. Restart a stopped
container with `docker start -ai ipdb_container`, or run the shell directly inside the container:

```bash
./build/debug/ipdb <your_database>
```

## Next steps

Continue to the [Quickstart](quickstart.md) to run your first prediction query, or
[Configuration](configuration.md) to set up API keys and backend-specific environment variables.
