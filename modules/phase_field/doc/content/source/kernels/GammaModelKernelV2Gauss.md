# GammaModelKernelV2Gauss

Kernel for $\gamma$-Model presented in [!cite](YEO2024127508)

## Overview

Implements the following term for the SECOND SET of grains:

\begin{equation}
-L\ m \nabla \cdot \left( \sum_{j \neq i} \left( \frac{\partial \gamma_{ij}}{\partial \nabla\eta_i} \right) \eta_i^2 \eta_j^2 \right)
\end{equation}

The anisotropic materials used in this kernel are computed strictly based on a limited set of potential grain boundaries. For example, if the system has three grains A, B, and C, all the determined potential grain boundaries would be AB, AC, and BC -in order.

All the Spherical Gaussian materials implement the following term that defines the time-changing boundary normal:
\begin{equation}
v_{ij} = \frac{\nabla \eta_i - \nabla \eta_j}{\|\nabla \eta_i - \nabla \eta_j\|}
\end{equation}
where $v_{ij}$ is the boundary normal between two specific grains / order  parameters $i$ and $j$.

Since the potential grain boundaries would be AB, AC, and BC -in order, we get two sets of parameters: $i$-{A,B} and $j$-{B,C}.

Considering that the derivative of the anisotropic parameter with respect to the gradient of the order parameter ($\frac{\partial \gamma_{ij}}{\partial \nabla\eta_i}$) depends on the time-changing boundary normal, the derivative would either be negative or positive since the material generates only one anisotropic parameter per pair of grains.

The Spherical Gaussian materials output both positive and negative derivatives. This kernel is tied to an action kernel that implements the second set with the negative derivative.

Since all the parameters and derivatives are defined continuously across the entire domain and tied to each boundary specifically, similarly to [!cite](MOELANS2022110592), there are no conflicts between kernels.


## Example Input File Syntax

A bicrystal input file is available. This can be used to reproduce the results in [!cite](YEO2024127508).

!listing modules/phase_field/test/tests/SphericalGaussian5DAnisotropyBicrystal/BicrystalGammaAnisoGauss.i

A tricrystal input file is available.

!listing modules/phase_field/examples/SphericalGaussian5DAnisotropyTricrystal/TricrystalGammaAnisoGauss.i


!syntax parameters /Kernels/GammaModelKernelV2Gauss

!syntax inputs /Kernels/GammaModelKernelV2Gauss

!syntax children /Kernels/GammaModelKernelV2Gauss
