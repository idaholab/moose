# JinSlabCoeffFunc

!syntax description /Functions/JinSlabCoeffFunc

## Overview

!style halign=left
This function object is used in the 1D Reflection Benchmark, in order to model
the incident wave. The complex-valued function is

\begin{equation}
  f(x) = k \sqrt{\epsilon_r \mu_r - \sin^2\left(\frac{\theta \pi}{180^{\circ}}\right)}
\end{equation}

where

- $k$ is the wave number ($2 \pi / \lambda$ where $\lambda$ is the wavelength),
- $\epsilon_r$ is the relative electric permittivity,
- $\mu_r$ is the relative magnetic permeability, and
- $\theta$ is the wave incidence angle, in degrees.

The relative electric permittivity is defined as

\begin{equation}
  \epsilon_r = 4 + (2 - j0.1)\left(\frac{1-kx}{5}\right)^2
\end{equation}

where $j = \sqrt{-1}$, and the relative magnetic permeability is defined as

\begin{equation}
  \mu_r = 2 - j0.1
\end{equation}

## Example Input File Syntax

!alert warning title=This is not currently tested
The JinSlabCoeffFunc object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /Functions/JinSlabCoeffFunc

!syntax inputs /Functions/JinSlabCoeffFunc

!syntax children /Functions/JinSlabCoeffFunc
