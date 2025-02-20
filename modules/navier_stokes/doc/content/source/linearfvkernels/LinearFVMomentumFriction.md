# LinearFVMomentumFriction

This kernel adds the friction term to the Navier Stokes momentum
equations. This kernel must be used with the canonical LinearFV variable set,
e.g. pressure and superficial velocity, and supports the Darcy friction model.


Darcy drag model

\begin{equation}
\label{darcy}
\epsilon F_i = - f_i \mu \epsilon \frac{v_{D,i}}{\epsilon} = -f_i \mu v_{D,i}
\end{equation}

where $F_i$ is the i-th component of the friction force (denoted by
$\mathbf{F_f}$ in [!eqref](LinearFV.md#eq:LinearFV_mom)), $f_i$ the friction factor,
which may be anisotropic, $\mu$ the fluid dynamic viscosity, $\rho$ the fluid density,
and $v_{D,i}$ the i-th component of the fluid superficial velocity.
We have used a negative sign to match the notation
used in [!eqref](LinearFV.md#eq:LinearFV_mom) where the friction force is on the
right-hand-side of the equation. When moved to the left-hand side, which is done
when setting up a Newton scheme, the term becomes positive which is what is
shown in the source code itself. Darcy is meant to represent viscous
effects and as shown in [darcy],[darcy2], it has a linear dependence on the fluid
velocity.

## Computation of friction factors and pre-factors id=friction_example

See the [PINSFVMomentumFriction.md] documentation for more information on the friction factors.

!syntax parameters /LinearFVKernels/LinearFVMomentumFriction

!syntax inputs /LinearFVKernels/LinearFVMomentumFriction

!syntax children /LinearFVKernels/LinearFVMomentumFriction
