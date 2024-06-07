# GAMMAandLanisoGAUSS

Anisotropic $\gamma$ and $L$

## Overview

This material is used to simulate the $\gamma$-Model presented in [!cite](Yeo2022) and [!cite](YEO2024127508). The $\gamma$-Model is a coupling of the Spherical-Gaussian Method from [!cite](BAIR2021110126) and the model by [!cite](MOELANS2022110592) through $\gamma$ and $L$.  

In this material, the Spherical-Gaussian Method is used to incorporate 5-D anisotropy to $\gamma$ and $L$. Mobility is variable and directly correlated to $L$. This allows the study of anisotropy through both grain boundary energy and grain boundary mobility.  

## Example Input File Syntax

A bicrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAandLGAUSS/BI_GAMMAandLGAUSS/BI_GAMMAandLGAUSS.i

A tricrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAandLGAUSS/TRI_GAMMAandLGAUSS/TRI_GAMMAandLGAUSS.i


!syntax parameters /Materials/GAMMAandLanisoGAUSS

!syntax inputs /Materials/GAMMAandLanisoGAUSS

!syntax children /Materials/GAMMAandLanisoGAUSS
