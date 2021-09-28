# ConstantDamper

!syntax description /Dampers/ConstantDamper

A constant damping factor may improve stability but will slow down convergence of the simulation.
More advanced dampers, which taper off through the iterations or which damp less within a certain
domain (such as the [BoundingValueNodalDamper.md]) should impact less convergence properties.

More information about dampers may be found on the
[Dampers syntax documentation](syntax/Dampers/index.md).

## Example input syntax

In this example, a constant damping factor of 0.9 is applied on all variables (just `u` here).

!listing test/tests/dampers/constant_damper/constant_damper_test.i block=Dampers

!syntax parameters /Dampers/ConstantDamper

!syntax inputs /Dampers/ConstantDamper

!syntax children /Dampers/ConstantDamper
