# PerfGraphReporter

## Description

The [/PerfGraph.md] holds timing and memory usage data for MOOSE applications. This Reporter stores the full performance graph from the [/PerfGraph.md] in the Reporter value `graph`.

The first and only entry in the value `graph` is the root node, where the key is the name of the root node. Each node in the graph contains the following entries:

- `level`: The level defined for the section that generated the node (see [PerfGraph.md#logging-levels])
- `memory`: The amount of memory (in Megabytes) that was allocated (or deallocated) within the node, not including children
- `num_calls`: The number of times the node was called
- `time`: The amount of time spent (in seconds) within the node, not including children

The remaining entries represent key value pairs for each of the children to said node.

When an application is run with multiple processes, the graph for each individual process is reported.

The [PerfGraphReporterReader](PerfGraphReporterReader.md optional=True) is a Python utility which can be used to rebuild and traverse the graph built by this reporter.

!syntax parameters /Reporters/PerfGraphReporter

!syntax inputs /Reporters/PerfGraphReporter

!syntax children /Reporters/PerfGraphReporter
