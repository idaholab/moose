# CommonOutputAction

!syntax description /Outputs/CommonOutputAction

## Overview

An action, that acts when the `[Outputs]` block exists. It adds short-cut syntax options, such as the
[!param](/Outputs/exodus) parameter, as well as common parameters that are applied to all output
objects. For example, the following enables two output objects and sets the output
[!param](/Outputs/interval) to every 10 timesteps for both objects.

```text
[Outputs]
  exodus = true
  csv = true
  interval = 10
[]
```

Please refer to the [syntax/Outputs/index.md] for more information.

!syntax parameters /Outputs/CommonOutputAction
