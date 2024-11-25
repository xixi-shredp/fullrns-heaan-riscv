CMD = ./FRNSHEAAN-native

perf:
	sudo perf record --call-graph dwarf $(CMD)

perf-rpt:
	flamegraph perf.data
	google-chrome perf.svg

.PHONY: perf perf-rpt
