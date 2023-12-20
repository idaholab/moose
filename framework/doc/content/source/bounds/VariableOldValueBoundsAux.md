# VariableOldValueBoundsAux

!syntax description /Bounds/VariableOldValueBoundsAux

## Description

`VariableOldValueBoundsAux` is used to set the bound for `bounded_variable` to be its old value. The bound type of `upper` or `lower` is set with `bound_type` parameter. `VariableOldValueBoundsAux` expects a `variable` parameter to be set (as do all `AuxKernels`). This can be a dummy
`AuxVariable`; the `VariableOldValueBoundsAux` actually operates on `NumericVectors` held by the
nonlinear system and does nothing but return 0 for the value of the specified
`variable`.

Note that in order for these bounds to have an effect, the user has to specify the
PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`. A warning will be generated if neither options are specified. MOOSE users can also specify bound with a constant using [`ConstantBoundsAux`](/ConstantBoundsAux.md).

## Example Syntax

!listing test/tests/bounds/old_value_bounds.i block=Bounds

!syntax parameters /Bounds/VariableOldValueBoundsAux

!syntax inputs /Bounds/VariableOldValueBoundsAux

!syntax children /Bounds/VariableOldValueBoundsAux
