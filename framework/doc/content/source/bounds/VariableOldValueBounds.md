# VariableOldValueBounds

!syntax description /Bounds/VariableOldValueBounds

## Description

`VariableOldValueBounds` is used to set the bound for `bounded_variable` to be its old value. The bound type of `upper` or `lower` is set with `bound_type` parameter. `VariableOldValueBounds` expects a `variable` parameter to be set (as do all `AuxKernels`). This can be a dummy
`AuxVariable`; the `VariableOldValueBounds` actually operates on `NumericVectors` held by the
nonlinear system and does nothing but return 0 for the value of the specified
`variable`.

Note that in order for these bounds to have an effect, the user has to specify the
PETSc options `-snes_type vinewtonssls` or `-snes_type vinewtonrsls`. A warning will be generated if neither option is specified. MOOSE users can also specify bound with a constant using [`ConstantBounds`](/ConstantBounds.md).

## Example Syntax

!listing test/tests/bounds/old_value_bounds.i block=Bounds

!syntax parameters /Bounds/VariableOldValueBounds

!syntax inputs /Bounds/VariableOldValueBounds

!syntax children /Bounds/VariableOldValueBounds
