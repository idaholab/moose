# Checkpoint

!syntax description /Outputs/Checkpoint

## Overview

The Checkpoint output object is designed to perform
[restart and recovery](restart_recover.md optional=True) of simulations. The output files
created contain a complete snapshot of the simulation. These files are required to perform
restart or recovery operations with a MOOSE-base application.

## Example Input Syntax

The simplest method for enabling checkpoint files is to use the short-cut syntax
(see [syntax/Outputs/index.md]) as follows. This will write checkpoint files at every timestep, but only
keep the most recent two sets of files to avoid excessive data storage.

```text
[Outputs]
  checkpoint = true
[]
```

To change the interval, the number of stored files, or any of the other parameters for the
Checkpoint object it is required to create a sub-block.

!listing checkpoint_interval.i block=Outputs


!syntax parameters /Outputs/Checkpoint

!syntax inputs /Outputs/Checkpoint

!syntax children /Outputs/Checkpoint
