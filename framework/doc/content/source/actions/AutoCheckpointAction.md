# AutoCheckpointAction

!syntax description /Outputs/AutoCheckpointAction

## Overview

An action, that acts when the `[Outputs]` block exists. It adds the
[!param](/Outputs/checkpoint) and [!param](/Outputs/disable_wall_time_checkpoints)
short-cut syntax parameters. For example, the following enables time_step_interval
based checkpoints while disabling wall time based checkpoints.

```text
[Outputs]
  checkpoint = true
  disable_wall_time_checkpoints = true
[]
```

Please refer to the [syntax/Outputs/index.md] for more information.

!syntax parameters /Outputs/AutoCheckpointAction
