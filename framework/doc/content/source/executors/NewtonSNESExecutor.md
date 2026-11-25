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
zero. The executor assembles the Jacobian block that couples system $i$ to system $j$ from all
residual objects of system $i$, keeping only the derivatives with respect to the dofs of system
$j$. Residual objects need no extra configuration. See [NMSMExecutor.md] for the preconditioned
Jacobian.

PETSc options for the multi-system outer solve use the executor name followed by an underscore
as their prefix, for example `-outer_snes_monitor` for an executor named `outer`.

!syntax parameters /Executors/NewtonSNESExecutor

!syntax inputs /Executors/NewtonSNESExecutor

!syntax children /Executors/NewtonSNESExecutor
