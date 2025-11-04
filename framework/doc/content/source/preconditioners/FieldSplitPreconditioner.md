# FSP

!syntax description /Preconditioning/FSP

## Overview

The `FieldSplitPreconditioner` allows for custom preconditioning for each nonlinear variable in the numerical
system. One or more variables may be targeted in a subsolve that will only consider part of the numerical system.
The preconditioning defined for these subsolves is used for the relevant block(s) in the global numerical system.

A `FSP` may for example be used for block-diagonal preconditioning by setting `full=false`
and no off-diagonal variable couplings. Numerical systems considering only a single variable
are then preconditioned individually. This is the default preconditioner for the `PJFNK` solves. See the
[Executioner documentation](Executioner/index.md) for more information on the default preconditioner.

More information about field split preconditioning may be found in the
[PETSc manual](https://petsc.org/release/manualpages/PC/PCFIELDSPLIT).

## Example input syntax

In this example, the preconditioning is performed by solving individual problems for each variables,
as described in the comments in the snippet. The solution for these subsolves is used to perform
the Schur decomposition preconditioning of the main numerical system.

!listing test/tests/preconditioners/fsp/fsp_test.i block=Preconditioning

An example of setting the [!param](/Preconditioning/FSP/off_diag_row) and
[!param](/Preconditioning/FSP/off_diag_column) parameters to create a custom
coupling matrix may be found in the
[PhysicsBasedPreconditioner.md] documentation.

!syntax parameters /Preconditioning/FSP

!syntax inputs /Preconditioning/FSP

!syntax children /Preconditioning/FSP
