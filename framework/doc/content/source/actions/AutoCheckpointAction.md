# AutoCheckpointAction

!syntax description /Outputs/AutoCheckpointAction

## Overview

An action, that acts when the `[Outputs]` block exists. It adds the
[!param](/Outputs/checkpoint) and [!param](/Outputs/wall_time_checkpoint)
short-cut syntax parameters. For example, the following enables time_step_interval
based checkpoints while disabling wall time based checkpoints.

```text
[Outputs]
  checkpoint = true
  wall_time_checkpoint = false
[]
```

Please refer to the [syntax/Outputs/index.md] for more information.

!syntax parameters /Outputs/AutoCheckpointAction
