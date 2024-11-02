# CpuCagra

**CPU Implementation of CAGRA Graph Index Building for Fast Approximate Nearest Neighbor Search**

## Overview

CpuCagra is a CPU-based implementation of the CAGRA graph index designed to perform efficient approximate nearest neighbor (ANN) searches. This implementation builds upon a K-Nearest Neighbor Graph (KNNG) to facilitate rapid search and retrieval tasks. The project relies on RapidJSON for parsing configuration files and can be easily configured and run with `run.sh`.


## Dependencies

- **CMake**: for building the project.
- **C++17 or later**: the project requires a compiler with C++17 or later support.

## Configuration

Configure your settings in `cagra.json` as shown below. This file includes paths for input and output graph data as well as key parameters for graph indexing:

### `cagra.json`

```json
{
    "cagra": {
        "KNNG_PATH": "/path/to/your/project-root/knng.graph",
        "KNNG_FORMAT": "efanna",
        "SAVE_PATH": "/path/to/your/project-root/cagra.graph",
        "R_INIT": 128,
        "R": 64
    }
}
```

### Configuration Options

Here's an improved version with your instructions:

---

### Configuration Options

- **KNNG_PATH**: Path to the input KNN graph.
- **SAVE_PATH**: Output path for the generated CAGRA graph.
- **KNNG_FORMAT**: Specifies the KNN-Graph file format, supporting both **efanna** and **fbin** formats.
    - **efanna**: Each entry consists of `k` (an unsigned 4-byte integer) followed by a list of `k` nearest neighbors (each represented by an unsigned 4-byte integer). This sequence is repeated for each node in the graph.
    - **fbin**: Each entry begins with `k` (an unsigned 4-byte integer) followed by `k` neighbor indices (each a 4-byte unsigned integer). The pattern continues for all nodes.
- **R_INIT**: Rank-based reorder graph degree parameter; must be less than or equal to the KNN graph degree.
- **R**: Final cagra graph degree parameter.


## Build and Run

### 1. Set Up Configuration

Ensure `cagra.json` is configured with the correct paths and parameters.

### 2. Run `run.sh`

The `run.sh` script will automatically build and execute the project. Ensure it has execute permissions:

```bash
chmod +x run.sh
./run.sh
```

## References
- [CAGRA: Highly Parallel Graph Construction and Approximate Nearest Neighbor Search](https://arxiv.org/abs/2308.15136)
