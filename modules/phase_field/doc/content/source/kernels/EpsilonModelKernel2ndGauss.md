# EpsilonModelKernel2ndGauss

Kernel for $\epsilon$-Model presented in [!cite](YEO2024127508)

## Overview

Implements the following term for the FIRST SET of grains:

\begin{equation}
-L\nabla \cdot \left( \frac{\partial m(\theta, v)}{\partial \nabla\eta_i} f_0 \right)
\end{equation}

The anisotropic materials used in this kernel are computed strictly based on a limited set of potential grain boundaries. For example, if the system has three grains A, B, and C, all the determined potential grain boundaries would be AB, AC, and BC -in order.

All the Spherical Gaussian materials implement the following term that defines the time-changing boundary normal:
\begin{equation}
v_{ij} = \frac{\nabla \eta_i - \nabla \eta_j}{\|\nabla \eta_i - \nabla \eta_j\|}
\end{equation}
where $v_{ij}$ is the boundary normal between two specific grains / order  parameters $i$ and $j$.

Since the potential grain boundaries would be AB, AC, and BC -in order, we get two sets of parameters: $i$-{A,B} and $j$-{B,C}.

Considering that the derivative of the anisotropic parameter with respect to the gradient of the order parameter ($\frac{\partial m(\theta, v)}{\partial \nabla \eta_i}$) depends on the time-changing boundary normal, the derivative would either be negative or positive since the material generates only one anisotropic parameter per pair of grains.

The Spherical Gaussian materials output both positive and negative derivatives. This kernel is tied to an action kernel that implements the first set with the positive derivative.

Since all the parameters and derivatives are defined continuously across the entire domain and tied to each boundary specifically, similarly to [!cite](MOELANS2022110592), there are no conflicts between kernels.


## Example Input File Syntax

A bicrystal input file is available. This can be used to reproduce the results in [!cite](YEO2024127508).

!listing modules/phase_field/test/tests/SphericalGaussian5DAnisotropyBicrystal/BicrystalEpsAndMAndLAnisoGauss.i

A tricrystal input file is available.

!listing modules/phase_field/examples/SphericalGaussian5DAnisotropyTricrystal/TricrystalEpsAndMAndLAnisoGauss.i


!syntax parameters /Kernels/EpsilonModelKernel2ndGauss

!syntax inputs /Kernels/EpsilonModelKernel2ndGauss

!syntax children /Kernels/EpsilonModelKernel2ndGauss
