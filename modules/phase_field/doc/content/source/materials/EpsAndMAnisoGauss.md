# EpsAndMAnisoGauss

Anisotropic $\epsilon$ (or also called $\kappa$) and $m$

Isotropic  $L$

## Overview

This material is used to simulate the $\epsilon$-Model presented in [!cite](Yeo2022) and [!cite](YEO2024127508). The $\epsilon$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\kappa$, $m$, and $L$.  

In this material, the Spherical-Gaussian Method is used to incorporate 5-D anisotropy to $\epsilon$ (or also called $\kappa$) and $m$. Mobility is assumed constant and directly correlated to $L$. This allows the study of anisotropy through grain boundary energy only; without considering the effect of grain boundary mobility.  

## Example Input File Syntax

A bicrystal input file is available. This can be used to reproduce the results in [!cite](YEO2024127508).

!listing modules/phase_field/test/tests/SphericalGaussian5DAnisotropyBicrystal/BicrystalEpsAndMAnisoGauss.i

A tricrystal input file is available.

!listing modules/phase_field/examples/SphericalGaussian5DAnisotropyTricrystal/TricrystalEpsAndMAnisoGauss.i


!syntax parameters /Materials/EpsAndMAnisoGauss

!syntax inputs /Materials/EpsAndMAnisoGauss

!syntax children /Materials/EpsAndMAnisoGauss
