# LinearFVMomentumFriction

This kernel adds the Darcy friction term to the Navier Stokes momentum
equations. This kernel must be used with the canonical linear finite volume discretization (`LinearFV`) variable set,
e.g. pressure and superficial velocity, and supports the Darcy friction model.

Darcy drag model

\begin{equation}
\label{darcy}
\epsilon F_i = - f_i \mu \epsilon v_i
\end{equation}

where $\epsilon$ is the phase fraction, $F_i$ is the i-th component of the friction force, $f_i$ the friction factor,
which may be anisotropic, $\mu$ the fluid dynamic viscosity, $\rho$ the fluid density,
and $v_i$ the i-th component of the fluid velocity.
We have used a negative sign as the friction force is on the
right-hand-side of the equation. Darcy is meant to represent viscous
effects and as shown in [darcy], it has a linear dependence on the fluid
velocity.

## Computation of friction factors and pre-factors id=friction_example

See the [PINSFVMomentumFriction.md] documentation for more information on the friction factors.

!syntax parameters /LinearFVKernels/LinearFVMomentumFriction

!syntax inputs /LinearFVKernels/LinearFVMomentumFriction

!syntax children /LinearFVKernels/LinearFVMomentumFriction
