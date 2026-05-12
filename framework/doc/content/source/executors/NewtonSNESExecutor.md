# NewtonSNESExecutor

!syntax description /Executors/NewtonSNESExecutor

## Overview

`NewtonSNESExecutor` runs Newton's method with a line search on one or more nonlinear systems.

For a single nonlinear system, the executor uses the system's own libMesh solver. If
[!param](/Executors/NewtonSNESExecutor/nl_preconditioning) is set, the executor attaches the
nonlinear preconditioner to that solver before each solve.

For multiple nonlinear systems, the executor builds an outer PETSc SNES on the combined solution
and residual. This path requires [!param](/Executors/NewtonSNESExecutor/nl_preconditioning). The
outer solve uses left nonlinear preconditioning, so it drives the preconditioner correction to
zero. The Jacobian blocks that couple two systems are assembled into matrices tagged
`NPC_J_<i>_<j>`, where `i` and `j` are the positions of the systems in
[!param](/Executors/NewtonSNESExecutor/nonlinear_system_names). Residual objects must add these
tags to contribute to the coupling blocks. See [NMSMExecutor.md] for the preconditioned Jacobian.

PETSc options for the multi-system outer solve use the executor name followed by an underscore
as their prefix, for example `-outer_snes_monitor` for an executor named `outer`.

!syntax parameters /Executors/NewtonSNESExecutor

!syntax inputs /Executors/NewtonSNESExecutor

!syntax children /Executors/NewtonSNESExecutor
