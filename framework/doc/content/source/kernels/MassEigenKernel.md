# MassEigenKernel

## Description

`MassEigenKernel` is used in the context of solving eigenvalue problems of the
form

\begin{equation}
\label{eq:general}
\widetilde{A}\vec{u} = \lambda\widetilde{B}\vec{u}
\end{equation}

where $\lambda$ represents the eigenvalue and $\vec{u}$ represents the
eigenvector. Kernels that inherit from `EigenKernel` like `MassEigenKernel` are
grouped into the $\widetilde{B}\vec{u}$ term and have their residual
contributions multiplied by the eigenvalue $\lambda$. Consider the equation below

\begin{equation}
\label{eq:specific}
-\nabla^2 u + ku = \underbrace{\lambda u}_{\textrm{MassEigenKernel}}
\end{equation}

where $k$ is some reaction coefficient that accounts for loss of $u$. The terms
on the LHS of [eq:specific] are equivalent to the LHS of [eq:general]
and similarly for the right hand sides.

## Example Syntax

The kernels describing [eq:specific] are shown in the `Kernel` block below

!listing test/tests/executioners/eigen_executioners/ipm.i start=[Kernels] end=[BCs]

The syntax for `MassEigenKernel` is simple, taking only its type and the
variable that the kernel acts on.

!syntax parameters /Kernels/MassEigenKernel

!syntax inputs /Kernels/MassEigenKernel

!syntax children /Kernels/MassEigenKernel
