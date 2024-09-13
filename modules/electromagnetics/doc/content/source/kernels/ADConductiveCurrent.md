# ADConductiveCurrent

!syntax description /Kernels/ADConductiveCurrent

## Overview

!style halign=left
The ADConductiveCurrent object implements a conduction current source term to the electric field Helmholtz wave equation. The term is define as:

\begin{equation}
  j \; \mu \; \omega \; \sigma \; \vec{E}
\end{equation}

where

- $j = \sqrt{-1}$,
- $\mu$ is the permeability of the medium,
- $\omega$ is the angular frequency of the wave propagation,
- $\sigma$ is the conductivity of the medium, and 
- $\vec{E}$ is the electric field.

## Example Input File Syntax

!listing vector_conductive_current.i block=Kernels/conduction_real

!syntax parameters /Kernels/ADConductiveCurrent

!syntax inputs /Kernels/ADConductiveCurrent

!syntax children /Kernels/ADConductiveCurrent
