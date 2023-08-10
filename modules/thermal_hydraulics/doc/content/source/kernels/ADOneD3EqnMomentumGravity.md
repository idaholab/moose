# ADOneD3EqnMomentumGravity

!syntax description /Kernels/ADOneD3EqnMomentumGravity

The force term from gravity in the momentum equation strong form is:

!equation
\rho A \vec{g} \cdot \vec{d}

where $\rho$ is the density, $A$ the area of the component, $\vec{g}$ the gravity vector and $\vec{d}$ the
direction of the flow component.

!alert note
In THM, most kernels are added automatically by components or flow models. This kernel is created by the
[FlowModelSinglePhase.md] to act inside components with single-phase fluid flow.

!syntax parameters /Kernels/ADOneD3EqnMomentumGravity

!syntax inputs /Kernels/ADOneD3EqnMomentumGravity

!syntax children /Kernels/ADOneD3EqnMomentumGravity
