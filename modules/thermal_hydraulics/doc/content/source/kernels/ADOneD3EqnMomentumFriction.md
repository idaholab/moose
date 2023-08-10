# ADOneD3EqnMomentumFriction

!syntax description /Kernels/ADOneD3EqnMomentumFriction

The friction loss term in the momentum equation strong form is:

!equation
\dfrac{1}{2 D_h} f_D \rho u |u| A

where $\rho$ is the density, $A$ the area of the component, $u$ the one-dimensional velocity, $D_h$
the hydraulic diameter and $f_D$ the Darcy friction factor.

!alert note
In THM, most kernels are added automatically by components or flow models. This kernel is created by the
[FlowModelSinglePhase.md] to act inside components with single-phase fluid flow.

!syntax parameters /Kernels/ADOneD3EqnMomentumFriction

!syntax inputs /Kernels/ADOneD3EqnMomentumFriction

!syntax children /Kernels/ADOneD3EqnMomentumFriction
