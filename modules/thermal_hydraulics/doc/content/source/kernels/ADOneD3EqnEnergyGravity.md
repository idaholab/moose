# ADOneD3EqnEnergyGravity

!syntax description /Kernels/ADOneD3EqnEnergyGravity

The local work term from the action of gravity in the energy equation strong form is:

!equation
\rho u A \vec{g} \cdot \vec{d}

where $\rho$ is the density, $u$ the one-dimensional velocity, $A$ the area of the component, $\vec{g}$ the gravity vector
and $\vec{d}$ the direction of the flow component.

!alert note
In THM, most kernels are added automatically by components or flow models. This kernel is created by the
[FlowModelSinglePhase.md] to act inside components with single-phase fluid flow.

!syntax parameters /Kernels/ADOneD3EqnEnergyGravity

!syntax inputs /Kernels/ADOneD3EqnEnergyGravity

!syntax children /Kernels/ADOneD3EqnEnergyGravity
