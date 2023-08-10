# OneD3EqnMomentumGravity

!syntax description /Kernels/OneD3EqnMomentumGravity

The force term from gravity in the momentum equation strong form is:

!equation
\rho A \vec{g} \cdot \vec{d}

where $\rho$ is the density, $A$ the area of the component, $\vec{g}$ the gravity vector and $\vec{d}$ the
direction of the flow component.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneD3EqnMomentumGravity.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneD3EqnMomentumGravity

!syntax inputs /Kernels/OneD3EqnMomentumGravity

!syntax children /Kernels/OneD3EqnMomentumGravity
