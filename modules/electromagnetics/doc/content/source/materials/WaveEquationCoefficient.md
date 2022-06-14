# WaveEquationCoefficient

!syntax description /Materials/WaveEquationCoefficient

## Overview

!style halign=left
This object provides a ready-to-use coefficient for the electric field Helmholtz
wave equation problem, specifically a coefficient material property of the form

\begin{equation}
  f(\mathbf{r}) = k^2 \mu_r \epsilon_r
\end{equation}

where

- $k$ is the complex wave number ($2 \pi / \lambda$ where $\lambda$ is the wavelength),
- $\epsilon_r$ is the complex relative electric permittivity, and
- $\mu_r$ is the complex relative magnetic permeability.

Note that all of these parameters (real and imaginary parts) can be provided by
the user as material properties.

## Example Input File Syntax

!listing scalar_complex_helmholtz.i block=Materials/wave_equation_coefficient

!syntax parameters /Materials/WaveEquationCoefficient

!syntax inputs /Materials/WaveEquationCoefficient

!syntax children /Materials/WaveEquationCoefficient
