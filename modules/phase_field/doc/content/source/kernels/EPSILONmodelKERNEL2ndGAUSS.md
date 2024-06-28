# EPSILONmodelKERNEL2ndGAUSS

Kernel for $\epsilon$-Model presented in [!cite](YEO2024127508)

## Overview

Implements the term:

\begin{equation}
-L\nabla \cdot \left( \frac{\partial m(\theta, v)}{\partial \nabla\eta_i} f_0 \right)
\end{equation}

## Example Input File Syntax

A bicrystal example input file is available. This can be used to reproduce the results in [!cite](YEO2024127508).

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/EPSILON_MODEL/EPSandMGAUSS/BI_EPSandMGAUSS/BI_EPSandMGAUSS.i

A tricrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/EPSILON_MODEL/EPSandMGAUSS/TRI_EPSandMGAUSS/TRI_EPSandMGAUSS.i

A bicrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/EPSILON_MODEL/EPSandMandLGAUSS/BI_EPSandMandLGAUSS/BI_EPSandMandLGAUSS.i

A tricrystal example input file is available.

!listing modules/phase_field/examples/Spherical-Gaussian_5D_anisotropy/EPSILON_MODEL/EPSandMandLGAUSS/TRI_EPSandMandLGAUSS/TRI_EPSandMandLGAUSS.i


!syntax parameters /Kernels/EPSILONmodelKERNEL2ndGAUSS

!syntax inputs /Kernels/EPSILONmodelKERNEL2ndGAUSS

!syntax children /Kernels/EPSILONmodelKERNEL2ndGAUSS
