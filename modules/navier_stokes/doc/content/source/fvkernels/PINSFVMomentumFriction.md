# PINSFVMomentumFriction

This kernel adds the friction term to the porous media Navier Stokes
momentum equations. This kernel should be use with a superficial velocity variable.
A variety of models are supported for the friction force:

Linear friction, for laminar flow
\begin{equation}
F_i = - f v_i
\end{equation}
Quadratic friction, for turbulent flow
\begin{equation}
F_i = - f v_i |v_i|
\end{equation}
Darcy drag model
\begin{equation}
F_i = - \dfrac{f_i}{\epsilon} \rho v_i
\end{equation}
Forchheimer drag model
\begin{equation}
F_i = - \dfrac{f_i}{\epsilon} \rho v_i
\end{equation}
where $F_i$ is the i-th component of the friction force, f the friction factor, which may be anisotropic,
$\epsilon$ the porosity and $\rho$ the fluid density and $v_i$ the fluid superficial velocity. The
Darcy and Forchheimer drags may also be expressed with a momentum material property using the
`momentum_name` parameter.

!syntax parameters /FVKernels/PINSFVMomentumFriction

!syntax inputs /FVKernels/PINSFVMomentumFriction

!syntax children /FVKernels/PINSFVMomentumFriction
