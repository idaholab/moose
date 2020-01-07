# Contact Action

## Description

`ContactAction` can be used to specify mechanical normal and tangential contact using several possible
models:
- frictionless
- glued
- coulomb (frictional)
and formulations:
- kinematic
- penalty (normal and tangential)
- augmented lagrange
- mortar (normal with/without tangential)

## Example Input syntax id=example

For normal mortar contact:

!listing test/tests/mechanical-small-problem/frictionless-nodal-lm-mortar-disp-action.i block=Contact

For normal and tangential (frictional) mortar contact:

!listing test/tests/bouncing-block-contact/frictional-nodal-min-normal-lm-mortar-fb-tangential-lm-mortar-action.i block=Contact

!syntax parameters /Contact/ContactAction

!syntax inputs /Contact/ContactAction

!syntax children /Contact/ContactAction
