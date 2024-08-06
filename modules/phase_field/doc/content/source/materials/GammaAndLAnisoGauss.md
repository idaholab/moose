# GammaAndLAnisoGauss

Anisotropic $\gamma$ and $L$

## Overview

This material is used to simulate the $\gamma$-Model presented in [!cite](Yeo2022) and [!cite](YEO2024127508). The $\gamma$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\gamma$ and $L$.  

In this material, the Spherical-Gaussian Method is used to incorporate 5-D anisotropy to $\gamma$ and $L$. Mobility is variable and directly correlated to $L$. This allows the study of anisotropy through both grain boundary energy and grain boundary mobility.  

## Example Input File Syntax

A bicrystal input file is available.

!listing modules/phase_field/test/tests/SphericalGaussian5DAnisotropyBicrystal/BicrystalGammaAndLAnisoGauss.i

A tricrystal input file is available.

!listing modules/phase_field/examples/SphericalGaussian5DAnisotropyTricrystal/TricrystalGammaAndLAnisoGauss.i


!syntax parameters /Materials/GammaAndLAnisoGauss

!syntax inputs /Materials/GammaAndLAnisoGauss

!syntax children /Materials/GammaAndLAnisoGauss
