# FullSolveMultiApp

!syntax description /MultiApps/FullSolveMultiApp

## Overview

The FullSolveMultiApp object is designed to perform a complete simulation every time it executes,
thus it can be used for creating simulations that have portions that are on drastically different
time scales.

## Checkpoint Recovery

When a simulation is run with the `--recover` flag , the parent application
restores its state from a checkpoint and continues execution. Because
`FullSolveMultiApp` always performs a complete simulation from scratch on every
execution, propagating the parent's recover state to its sub-applications would
cause incorrect behavior: the sub-application would attempt to resume from a
checkpointed end-state rather than re-solving from the beginning.

To prevent this, `FullSolveMultiApp` never propagates the `--recover` flag to
its sub-applications. Each time the parent calls the `FullSolveMultiApp`, the
sub-application performs a fresh, complete solve regardless of whether the
parent is recovering. This behavior applies recursively: sub-applications of a
`FullSolveMultiApp` will also not propagate these flags to their own
sub-applications, ensuring that the entire subtree below a `FullSolveMultiApp`
boundary always re-solves completely.

## Example Input File Syntax

The following code snippet demonstrates how to create a FullSolveMultiApp object.

!listing full_solve_multiapp/parent.i block=MultiApps


!syntax parameters /MultiApps/FullSolveMultiApp

!syntax inputs /MultiApps/FullSolveMultiApp

!syntax children /MultiApps/FullSolveMultiApp
