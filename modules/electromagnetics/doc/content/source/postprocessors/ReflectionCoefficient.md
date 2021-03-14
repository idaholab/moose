# ReflectionCoefficient

!syntax description /UserObjects/ReflectionCoefficient

## Overview

!style halign=left
This object is used within the [OneDReflection.md] in order to calculate the
resulting reflection coefficient of the incoming wave. This assumes that the
complex-valued solution wave at the domain boundary has the form

\begin{equation}
  F_{boundary} = F_{incoming} + R F_{reflected}
\end{equation}

where $R$ is the reflection coefficient. As the wave is a complex-valued plane
wave in the benchmark case, the incoming and reflected plane waves have the
general forms

\begin{equation}
  F_{incoming} = C e^{jkL\cos(\theta \pi / 180^{\circ})} \\
  F_{reflected} = C e^{-jkL\cos(\theta \pi / 180^{\circ})}
\end{equation}

where

- $C$ is a constant coefficient representing the amplitude of the incoming wave,
- $j = \sqrt{-1}$,
- $k$ is the wave number ($2 \pi / \lambda$ where $\lambda$ is the wavelength),
- $L$ is the length of the slab domain, and
- $\theta$ is the incident angle of the incoming wave.

## Example Input File Syntax

!alert warning title=This is not currently tested
The ReflectionCoefficient object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /UserObjects/ReflectionCoefficient

!syntax inputs /UserObjects/ReflectionCoefficient

!syntax children /UserObjects/ReflectionCoefficient
