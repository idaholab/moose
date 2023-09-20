# Progress

!syntax description /Outputs/Progress

## Overview

The Progress output displays an ASCII art progress bar at the end of each timestep, visualizing the amount of simulation time that has passed vs. the total simulation time. It requires the use of a transient executioner along with predetermined start and end times. The width of the bar widget can be specified using the [!param](/Outputs/Progress/progress_bar_width) parameter. If omitted the value of the `MOOSE_PPS_WIDTH` environment variable is queried. If that variable is not set the terminal window width is queried (with a fallback value of 132 chars).

```
+-Progress (full.i)--------------------------------+
|#########################.........................|
+--------------------------------------------------+
```

!syntax parameters /Outputs/Progress

!syntax inputs /Outputs/Progress

!syntax children /Outputs/Progress
