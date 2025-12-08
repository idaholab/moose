# PerfGraphReporterReader

A python utility that provides an interface for reading [PerfGraphReporter.md] output. It rebuilds the graph for easy traversal via the `PerfGraphNode` and `PerfGraphSection` objects.

## Example usage

Take the following simple diffusion problem, which has a [PerfGraphReporter.md] set to output on final:

!listing test/tests/reporters/perf_graph_reporter/perf_graph_reporter.i

For more real-world-like timing, we will pass the command line arguments "`Mesh/gmg/nx=500 Mesh/gmg/ny=500`", as the test above is executed with only a single element. With this, we will execute the following (where `MOOSE_DIR` is an environment variable set to the directory that contains MOOSE):

!listing
$MOOSE_DIR/test/moose_test-opt -i $MOOSE_DIR/test/tests/reporters/perf_graph_reporter/perf_graph_reporter.i Mesh/gmg/nx=500 Mesh/gmg/ny=500

This run will generate the desired output in `$MOOSE_DIR/test/tests/reporters/perf_graph_reporter/perf_graph_reporter_json.json`.

Load a `PerfGraphReporterReader` with the given output as such:

!listing language=python
import os
from mooseutils.PerfGraphReporterReader import PerfGraphReporterReader
MOOSE_DIR = os.environ.get('MOOSE_DIR')
pgrr = PerfGraphReporterReader(MOOSE_DIR + '/test/tests/reporters/perf_graph_reporter/perf_graph_reporter_json.json')
