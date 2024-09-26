# GammaModelGammaGradientKernel

Kernel for $\gamma$-Model presented in [!cite](YEO2024127508)

## Overview

Implements the following term for both FIRST and SECOND sets of grains:

\begin{equation}
-L\ m \nabla \cdot \left( \sum_{j \neq i} \left( \frac{\partial \gamma_{ij}}{\partial \nabla\eta_i} \right) \eta_i^2 \eta_j^2 \right)
\end{equation}

The anisotropic material used in this kernel is computed strictly based on a limited set of potential grain boundaries. For example, if the system has three grains A, B, and C, all the determined potential grain boundaries would be AB, AC, and BC -in order.

The Spherical Gaussian material [SphericalGaussianMaterial.md] implements the following term that defines the time-changing boundary normal:
\begin{equation}
v_{ij} = \frac{\nabla \eta_i - \nabla \eta_j}{\|\nabla \eta_i - \nabla \eta_j\|}
\end{equation}
where $v_{ij}$ is the boundary normal between two specific grains / order  parameters $i$ and $j$.

Since the potential grain boundaries would be AB, AC, and BC -in order, we get two sets of parameters: $i$-{A,B} and $j$-{B,C}.

Considering that the derivative of the anisotropic parameter with respect to the gradient of the order parameter ($\frac{\partial \gamma_{ij}}{\partial \nabla\eta_i}$) depends on the time-changing boundary normal, the derivative would either be negative or positive since the material generates only one anisotropic parameter per pair of grains.

The Spherical Gaussian material [SphericalGaussianMaterial.md] outputs both positive and negative derivatives. This kernel is tied to an action kernel [SphericalGaussianKernelAction.md] that implements both FIRST and SECOND sets.

Since all the parameters and derivatives are defined continuously across the entire domain and tied to each boundary specifically, similarly to [!cite](MOELANS2022110592), there are no conflicts between kernels.


## Example Input File Syntax

A bicrystal input file is available for the $\gamma$-Model. The kernel is implemented through the kernel action [SphericalGaussianKernelAction.md]:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/gamma_model_bicrystal.i

A tricrystal input file is available for the $\gamma$-Model. The kernel is implemented through the kernel action [SphericalGaussianKernelAction.md]:

!listing modules/phase_field/examples/spherical_gaussian_tricrystal/gamma_model_tricrystal.i


!syntax parameters /Kernels/GammaModelGammaGradientKernel

!syntax inputs /Kernels/GammaModelGammaGradientKernel

!syntax children /Kernels/GammaModelGammaGradientKernel
