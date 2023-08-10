# OneD3EqnEnergyGravity

!syntax description /Kernels/OneD3EqnEnergyGravity

The local work term from the action of gravity in the energy equation strong form is:

!equation
\rho u A \vec{g} \cdot \vec{d}

where $\rho$ is the density, $u$ the one-dimensional velocity, $A$ the area of the component, $\vec{g}$ the gravity vector
and $\vec{d}$ the direction of the flow component.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneD3EqnEnergyGravity.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneD3EqnEnergyGravity

!syntax inputs /Kernels/OneD3EqnEnergyGravity

!syntax children /Kernels/OneD3EqnEnergyGravity
