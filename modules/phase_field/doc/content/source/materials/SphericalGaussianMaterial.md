# SphericalGaussianMaterial

Anisotropic $\epsilon$ - (or also called $\kappa$), $m$ - (or also called mu), $\gamma$ and $L$

## Overview

This material is used to simulate the $\epsilon$-Model and $\gamma$-Model presented in [!cite](Yeo2022) and [!cite](YEO2024127508).

The $\epsilon$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\kappa$, $m$, and $L$.  

The $\gamma$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\gamma$ and $L$.

Both models allow the study of anisotropy through grain boundary energy and grain boundary mobility using a kernel action [SphericalGaussianKernelAction.md].  

## Example Input File Syntax

A bicrystal input file is available for the $\epsilon$-Model:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/epsilon_model_bicrystal.i

A tricrystal input file is available for the $\epsilon$-Model:

!listing modules/phase_field/examples/spherical_gaussian_tricrystal/epsilon_model_tricrystal.i

A bicrystal input file is available for the $\gamma$-Model:

!listing modules/phase_field/test/tests/spherical_gaussian_bicrystal/gamma_model_bicrystal.i

A tricrystal input file is available for the $\gamma$-Model:

!listing modules/phase_field/examples/spherical_gaussian_tricrystal/gamma_model_tricrystal.i


!syntax parameters /Materials/SphericalGaussianMaterial

!syntax inputs /Materials/SphericalGaussianMaterial

!syntax children /Materials/SphericalGaussianMaterial
