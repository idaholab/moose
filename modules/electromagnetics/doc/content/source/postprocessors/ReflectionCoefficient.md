# ReflectionCoefficient

!syntax description /Postprocessors/ReflectionCoefficient

## Overview

!style halign=left
This object is used within the [OneDReflection.md] in order to calculate the
resulting reflection coefficient of the incoming wave. This assumes that the
complex-valued solution wave at the domain boundary has the form

\begin{equation}
  F_{boundary} = F_{incoming} + R F_{reflected}
\end{equation}

where $R$ is the reflection coefficient of the wave. As the wave is a
complex-valued plane wave in the benchmark case, the incoming and reflected plane
waves have the general forms

\begin{equation}
  F_{incoming} = C e^{jkL\cos(\theta \pi / 180^{\circ})} \\
  F_{reflected} = C e^{-jkL\cos(\theta \pi / 180^{\circ})}
\end{equation}

where

- $C$ is a constant coefficient representing the amplitude of the incoming wave,
- $j = \sqrt{-1}$,
- $k$ is the wave number ($2 \pi / \lambda$ where $\lambda$ is the wavelength),
- $L$ is the length of the slab domain, and
- $\theta$ is the incident angle of the incoming wave, in degrees.

To calculate the percentage of reflected power, as required in the benchmark,
the squared magnitude of $R$ above is taken as the object output

!equation
R_{power} = |R|^2

## Example Input File Syntax

!listing slab_reflection.i block=Postprocessors/reflection_coefficient

!syntax parameters /Postprocessors/ReflectionCoefficient

!syntax inputs /Postprocessors/ReflectionCoefficient

!syntax children /Postprocessors/ReflectionCoefficient
