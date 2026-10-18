# RigidBodyNormalMechanicalContact

!syntax description /BCs/RigidBodyNormalMechanicalContact

## Description

Transfers the nodal contact multiplier field $\lambda$ on the lower-d
contact block back to a chosen displacement component on the parent
block through the integral

!equation
r_i^{d,\,\text{contact}}
  = -\int_{\Gamma_c} \phi_i\,\lambda\,\hat{n}\cdot\hat{e}_d\ \mathrm{d}\Gamma,

using the contactor's outward normal at the deformed quadrature point.
One instance of this boundary condition is emitted per displacement
component by [RigidContactAction.md].

The cross-node coupling between an LM DoF at one node and a
displacement DoF at an adjacent node on the same lower-d element must
be preallocated by [RigidBodyContactSparsity.md]; see the
[theory page](modules/contact/rigid_contact/theory.md#force-transfer-to-the-deformable-body).

## Parameters

!syntax parameters /BCs/RigidBodyNormalMechanicalContact
