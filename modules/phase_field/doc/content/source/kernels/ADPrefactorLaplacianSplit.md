# ADPrefactorLaplacianSplit

!syntax description /Kernels/ADPrefactorLaplacian

This kernel implements a laplacian kernel as follows:

\begin{equation}
    prefactor \nabla^2 \psi
\end{equation}

where, $\psi$ is the variable to which the laplacian operator is applied to and $prefactor$ is the coefficient.

# Example Syntax

The `ADPrefactorLaplacianSplit` kernel can used for a transient diffusion problem with the diffusivity as a prefactor.

!listing modules/phase_field/test/tests/ADLaplacianOperator/ADPrefactorLaplacianSplit/ad_transient_prefactorLaplacianSplit.i block=Kernels


!syntax parameters /Kernels/ADPrefactorLaplacianSplit

!syntax inputs /Kernels/ADPrefactorLaplacianSplit

!syntax children /Kernels/ADPrefactorLaplacianSplit
