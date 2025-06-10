#!/bin/bash

/opt/perf/pmu-tools/toplev.py -l3 -v --no-desc taskset -c 0 "$@"
