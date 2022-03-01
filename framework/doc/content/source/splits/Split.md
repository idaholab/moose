# Split

!syntax description /Preconditioning/Split

This is an alternative syntax to the [FieldSplitPreconditioner.md].
The field split based preconditioner is entirely set up by defining the relevant PETSc options.
This syntax offers shorthands for some of the parameters that may be passed to PETSc,
such as for splitting between variables, blocks or sidesets.

More information about field split preconditioning may be found in the
[PETSc manual]([here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/).

## Example input syntax

In this example, a two level field split preconditioning is set up using the `Split` syntax.
A Schur decomposition is used for preconditioning at the top level, and `asm` preconditioning,
with their own sets of PETSc options, is set set up for two groups of variables.

!listing modules/phase_field/test/tests/phase_field_crystal/PFCTrad/pfct_newton_split1_asm5.i block=Splits

!syntax parameters /Preconditioning/Split

!syntax inputs /Preconditioning/Split

!syntax children /Preconditioning/Split
