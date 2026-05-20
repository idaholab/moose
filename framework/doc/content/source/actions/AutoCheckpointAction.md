# AutoCheckpointAction

!syntax description /Outputs/AutoCheckpointAction

## Overview

An action, that acts when the `[Outputs]` block exists. It adds the
[!param](/Outputs/checkpoint)
short-cut syntax parameter. For example, the following adds a checkpoint
object that outputs every time step (default interval criterion) and every
one hour (default wall-time criterion).

```text
[Outputs]
  checkpoint = true
[]
```

Please refer to the [syntax/Outputs/index.md] for more information.

!syntax parameters /Outputs/AutoCheckpointAction
