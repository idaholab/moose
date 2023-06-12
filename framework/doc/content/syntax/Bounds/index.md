# Bounds System

The `Bounds` system is designed to bound the value of a nonlinear variable. Whether the bound is an upper or lower
bound depends on the parameters passed to the `Bounds` object. The bound may be spatially and time-dependent,
and even depend on other simulation quantities, as implemented in the particular `Bounds` object used.

The auxiliary variable that serves as the `variable` parameter of a `Bounds` object
is not actually used or even set in the computation. However, its type is used to decide if the `Bounds` loop
will be 'nodal' (loop on all nodes) or 'elemental' (loop on quadrature points in elements). Its block restriction
is used to define where the bounds is applied. It may be re-used for multiple bounds objects.

!alert note
Only nodal and constant elemental variables are supported at this time.

The `Bounds` system supports both finite element and finite volume variables. Only elemental bounds
should be used for finite volume variables.

Note that in order for `Bounds` to have an effect, the user has to specify the
PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`. A warning will be generated if neither option is specified. The PETSc manual pages for the `vinewtonssls` algorithm
can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONSSLS.html)
while the manual page for `vinewtonrsls` can be found
[here](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/SNES/SNESVINEWTONRSLS.html#SNESVINEWTONRSLS).

## Example syntax

In the following example, a lower and an upper bound are applied to two variables `u` and `v`
using the same auxiliary variable `bounds_dummy` and four `Bounds` objects.

!listing test/tests/auxkernels/bounds/constant_bounds.i block=Variables AuxVariables Bounds

!syntax list /Bounds objects=True actions=False subsystems=False

!syntax list /Bounds objects=False actions=False subsystems=True

!syntax list /Bounds objects=False actions=True subsystems=False
