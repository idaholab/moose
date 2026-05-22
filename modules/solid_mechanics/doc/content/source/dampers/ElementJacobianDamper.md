# ElementJacobianDamper

!syntax description /Dampers/ElementJacobianDamper

This damper limits the change in the Jacobians of elements. The damper becomes active if the relative change in Jacobian of any element exceeds the user defined maximum. This damper must be run on the displaced mesh.

Set [!param](/Dampers/ElementJacobianDamper/use_backtracking) to true to have the damper
iteratively cut back a trial nonlinear update when probing. Without backtracking, a trial update
that creates a degenerate element map will immediately raise an exception.

!syntax parameters /Dampers/ElementJacobianDamper

!syntax inputs /Dampers/ElementJacobianDamper

!syntax children /Dampers/ElementJacobianDamper
