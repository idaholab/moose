# GAMMAanisoGAUSS

Anisotropic $\gamma$

Isotropic  $L$

## Overview

This material is used to simulate the $\gamma$-Model presented in [!cite](Yeo2022) and [!cite](YEO2024127508). The $\gamma$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\gamma$ and $L$.  

In this material, the Spherical-Gaussian Method is used to incorporate 5-D anisotropy to $\gamma$. Mobility is assumed constant and directly correlated to $L$. This allows the study of anisotropy through grain boundary energy only; without considering the effect of grain boundary mobility.  

## Example Input File Syntax

A bicrystal example input file is available. This can be used to reproduce the results in [!cite](YEO2024127508).

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAGAUSS/BI_GAMMAGAUSS/BI_GAMMAGAUSS.i

A tricrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAGAUSS/TRI_GAMMAGAUSS/TRI_GAMMAGAUSS.i


!syntax parameters /Materials/GAMMAanisoGAUSS

!syntax inputs /Materials/GAMMAanisoGAUSS

!syntax children /Materials/GAMMAanisoGAUSS
