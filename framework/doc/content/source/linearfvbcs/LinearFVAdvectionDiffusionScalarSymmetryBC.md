# LinearFVAdvectionDiffusionScalarSymmetryBC

## Description

This object specifies a symmetry boundary condition for a diffusion equation:

\begin{equation}
\vec{n}_b \cdot D_b \nabla u_b~ = 0.0.
\end{equation}

where the $\vec{n}_b$ is the boundary normal unit vector and $D_b$ is the diffusion
coefficient at the boundary.

The face value for the advective terms and gradient computations can be
computed with or without a linear extrapolation to the face:

\begin{equation}
u_b = u_C + (\vec{d}_{Cb}-(\vec{d}_Cb \cdot \vec{n}_b)\vec{n}_b)\nabla u_C
\end{equation}

where $\vec{d}_{Cb}$ is the vector from the cell center to the boundary face center,
while $u_C$ and $\nabla u_C$ denote the cell center and cell gradient values of $u$, respectively.

!alert note
- This boundary condition should only be applied to problems involving advection and/or
  diffusion.
- In most cases symmetry boundary conditions don't have cross-flows on the symmetry
  boundaries. Even though the current formulation can be used with boundaries with
  flows, it is not recommended.
- In its current formulation, the stencil achieves only first-order spatial
  convergence on meshes composed of triangles and tetrahedra.

!syntax parameters /LinearFVBCs/LinearFVAdvectionDiffusionScalarSymmetryBC

!syntax inputs /LinearFVBCs/LinearFVAdvectionDiffusionScalarSymmetryBC

!syntax children /LinearFVBCs/LinearFVAdvectionDiffusionScalarSymmetryBC
