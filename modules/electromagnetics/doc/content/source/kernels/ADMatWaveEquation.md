# ADMatWaveEquation

!syntax description /Kernels/ADMatWaveEquation

## Overview

!style halign=left
The ADMatWaveEquation object implements a displacement current source term to the electric field Helmholtz wave equation. The term is define as:

\begin{equation}
  -\omega^{2} \; \mu \; \varepsilon \; \vec{E}
\end{equation}

where

- $\omega$ is the angular frequency of the wave propagation,
- $\mu$ is the permeability of the medium,
- $\varepsilon$ is the permittivity of the medium, and 
- $\vec{E}$ is the electric field.

Note that $\omega^{2} \; \mu \; \varepsilon$ is provided via the Materials block, using the
[WaveEquationCoefficient](/materials/WaveEquationCoefficient.md) object.

## Example Input File Syntax

!listing vector_ADmaterial_wave_equation.i block=Kernels/coeff_real

!syntax parameters /Kernels/ADMatWaveEquation

!syntax inputs /Kernels/ADMatWaveEquation

!syntax children /Kernels/ADMatWaveEquation
