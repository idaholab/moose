# ElementJacobianDamper
!description /Dampers/ElementJacobianDamper

This damper limits the change in the Jacobians of elements. The damper becomes active if the relative change in Jacobian of any element exceeds the user defined maximum.

!parameters /Dampers/ElementJacobianDamper

!inputfiles /Dampers/ElementJacobianDamper
moose/modules/tensor_mechanics/tests/jacobian_damper/cube_load.i

!childobjects /Dampers/ElementJacobianDamper
