# WaveCoeff

!syntax description /Functions/WaveCoeff

## Overview

!style halign=left
This object provides a ready-to-use coefficient for the electric field Helmholtz wave equation problem, specifically a coefficient function of the form

\begin{equation}
  f(\mathbf{r}) = k^2 \mu_r \epsilon_r
\end{equation}

where

- $k$ is the wave number ($2 \pi / \lambda$ where $\lambda$ is the wavelength),
- $\epsilon_r$ is the complex relative electric permittivity, and
- $\mu_r$ is the complex relative magnetic permeability.

## Example Input File Syntax

!alert warning title=This is not currently tested
The WaveCoeff object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /Functions/WaveCoeff

!syntax inputs /Functions/WaveCoeff

!syntax children /Functions/WaveCoeff
