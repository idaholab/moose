# OneD3EqnMomentumFormLoss

!syntax description /Kernels/OneD3EqnMomentumFormLoss

The form loss term in the momentum equation strong form is:

!equation
\dfrac{1}{2} K' \rho u |u| A

where $\rho$ is the density, $A$ the area of the component, $u$ the one-dimensional velocity and
$K'$ a form loss factor.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneD3EqnMomentumFormLoss.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneD3EqnMomentumFormLoss

!syntax inputs /Kernels/OneD3EqnMomentumFormLoss

!syntax children /Kernels/OneD3EqnMomentumFormLoss
