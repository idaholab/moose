# PCNSFVMomentumFriction

This kernel adds the friction term to the fully compressible porous media Navier
Stokes momentum equations. This kernel requires that the parameter
`momentum_name` is provided.  Darcy and Forchheimer models, as well as the
superposition of the two, are supported for the friction force:

Darcy drag model
\begin{equation}
F_i = - \dfrac{d_i}{\epsilon} p_i
\end{equation}
Forchheimer drag model
\begin{equation}
F_i = - \dfrac{f_i}{\epsilon} \rho p_i
\end{equation}
where $F_i$ is the i-th component of the friction force, $d_i$ is the i-th
component of the Darcy friction factor,
$\epsilon$ is the porosity, $p_i$ is the ith-component of the momentum, and
$f_i$ is the ith-component of the Forchheimer friction factor. If both
`Darcy_name` and `Forchheimer_name` parameters are supplied, then the drag
forces from the two models will be summed. The component is determined by the
`momentum_component` parameter.

!syntax parameters /FVKernels/PCNSFVMomentumFriction

!syntax inputs /FVKernels/PCNSFVMomentumFriction

!syntax children /FVKernels/PCNSFVMomentumFriction
