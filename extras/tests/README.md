# Testing

This directory contains tools to test specific characteristics of the server, e.g. performance.

## Setup

Create the virtual environment with the required dependencies:

```bash
virtualenv --python=python3 env
source env/bin/activate
pip install -r requirements.txt
```

## Scripts

| Script | Description |
| ------ | ----------- |
| chart_perf_tests.py | **Visualizes performance tests** created by `run_perf_tests.py`. |
| run_perf_tests.py | **Runs performance tests**: Command line tool to request a bunch of resources from the server, with various clients synchronously, and one or more pooled connections for each client. Use `chart_perf_tests.py` to visualize the output (`./run_perf-tests.py ... > ./chart_perf_tests.py` in its simplest form) |
