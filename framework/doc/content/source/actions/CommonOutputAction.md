# CommonOutputAction

!syntax description /Outputs/CommonOutputAction

## Overview

An action, that acts when the `[Outputs]` block exists. It adds short-cut syntax options, such as the
[!param](/Outputs/exodus) parameter, as well as common parameters that are applied to all output
objects. For example, the following enables two output objects and sets the output
[!param](/Outputs/time_step_interval) to every 10 timesteps for both objects.

```text
[Outputs]
  exodus = true
  csv = true
  interval = 10
[]
```

Please refer to the [syntax/Outputs/index.md] for more information.

### JSON performance graph

The parameters [!param](/Outputs/perf_graph_json) and [!param](/Outputs/perf_graph_json_file) can be utilized to conveniently output the [PerfGraph.md] to a JSON file. This enables systematic collection of a solve's performance into a database. Internally, these parameters create a [PerfGraphReporter.md] reporter and then output it using a [JSONOutput.md] output object.

When using the [!param](/Outputs/perf_graph_json) option, the file is output to `<file_base>_perf_graph.json`, where `<file_base>` is the standard application output file base (which can be overridden by [!param](/Outputs/file_base)). When using the [!param](/Outputs/perf_graph_json_file) option, a path to a `.json` file should be explicitly provided.

!syntax parameters /Outputs/CommonOutputAction
