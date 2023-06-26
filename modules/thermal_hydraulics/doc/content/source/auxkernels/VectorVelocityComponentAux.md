# VectorVelocityComponentAux

This object computes a component of a vector-valued velocity field.

The vector velocity is computed as:
\begin{equation}
\vec{u} = \frac{\rho u A}{\rho A} \vec{d}
\end{equation}
$\vec{u}$ is velocity vector, $\vec{d}$ is the direction of the element, $\rho A$ is density multiplied by cross-sectional area (also called conserved density), and $\rho u A$ is momentum density multiplied by cross-sectional area (also called conserved momentum).

!alert note
$\rho u A$ and $\rho A$ are variables usually defined by the [Components](syntax/Components/index.md).

!syntax parameters /AuxKernels/VectorVelocityComponentAux

!syntax inputs /AuxKernels/VectorVelocityComponentAux

!syntax children /AuxKernels/VectorVelocityComponentAux
