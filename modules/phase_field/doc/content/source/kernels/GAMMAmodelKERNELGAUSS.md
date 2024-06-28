# GAMMAmodelKERNELGAUSS

Kernel for $\gamma$-Model presented in [!cite](YEO2024127508)

## Overview

Implements the term:

\begin{equation}
-L\ m \nabla \cdot \left( \sum_{j \neq i} \left( \frac{\partial \gamma_{ij}}{\partial \nabla\eta_i} \right) \eta_i^2 \eta_j^2 \right)
\end{equation}

## Example Input File Syntax

A bicrystal example input file is available. This can be used to reproduce the results in [!cite](YEO2024127508).

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAGAUSS/BI_GAMMAGAUSS/BI_GAMMAGAUSS.i

A tricrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAGAUSS/TRI_GAMMAGAUSS/TRI_GAMMAGAUSS.i

A bicrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAandLGAUSS/BI_GAMMAandLGAUSS/BI_GAMMAandLGAUSS.i

A tricrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/GAMMA_MODEL/GAMMAandLGAUSS/TRI_GAMMAandLGAUSS/TRI_GAMMAandLGAUSS.i


!syntax parameters /Kernels/GAMMAmodelKERNELGAUSS

!syntax inputs /Kernels/GAMMAmodelKERNELGAUSS

!syntax children /Kernels/GAMMAmodelKERNELGAUSS
