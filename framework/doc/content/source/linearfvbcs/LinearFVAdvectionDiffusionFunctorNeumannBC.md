# LinearFVAdvectionDiffusionFunctorNeumannBC

## Description

`LinearFVAdvectionDiffusionFunctorNeumannBC` specifies the diffusive flux at the boundary.
The value will be determined by a `Functor`
(through the [!param](/LinearFVBCs/LinearFVAdvectionDiffusionFunctorNeumannBC/functor) parameter).

Specifically, the specified functor defines the following boundary flux for variable $u$

\begin{equation}
\vec{n}_b \cdot D_b \nabla u_b~,
\end{equation}

where the $\vec{n}_b$ is the boundary normal unit vector and $D_b$ is the diffusion coefficient at the boundary.

!alert note
This boundary condition should only be used for problems which involve advection and/or diffusion
problems.

!syntax parameters /LinearFVBCs/LinearFVAdvectionDiffusionFunctorNeumannBC

!syntax inputs /LinearFVBCs/LinearFVAdvectionDiffusionFunctorNeumannBC

!syntax children /LinearFVBCs/LinearFVAdvectionDiffusionFunctorNeumannBC
