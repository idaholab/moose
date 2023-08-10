# OneD3EqnMomentumFriction

!syntax description /Kernels/OneD3EqnMomentumFriction

The friction loss term in the momentum equation strong form is:

!equation
\dfrac{1}{2 D_h} f_D \rho u |u| A

where $\rho$ is the density, $A$ the area of the component, $u$ the one-dimensional velocity, $D_h$
the hydraulic diameter and $f_D$ the Darcy friction factor.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneD3EqnMomentumFriction.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneD3EqnMomentumFriction

!syntax inputs /Kernels/OneD3EqnMomentumFriction

!syntax children /Kernels/OneD3EqnMomentumFriction
