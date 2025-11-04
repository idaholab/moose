# ADLaplacianSplit

!syntax description /Kernels/ADLaplacianSplit

This kernel implements a laplacian kernel as follows:

\begin{equation}
    \nabla^2 \psi
\end{equation}

where, $\psi$ is the variable to which the laplacian operator is applied to.

# Example Syntax

The `ADLaplacianSplit` kernel can used for a diffusion problem with a body force as shown below

!listing modules/phase_field/test/tests/ADLaplacianOperator/ADLaplacianSplit/2D_ADLaplacianSplit_bodyforce.i block=Kernels


!syntax parameters /Kernels/ADLaplacianSplit

!syntax inputs /Kernels/ADLaplacianSplit

!syntax children /Kernels/ADLaplacianSplit
