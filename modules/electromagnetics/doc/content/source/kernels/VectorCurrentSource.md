# VectorCurrentSource

!syntax description /Kernels/VectorCurrentSource

## Overview

!style halign=left
The VectorCurrentSource object implements the volumetric current source term in
the electric field Helmholtz wave equation. This term, in general, is

\begin{equation}
  j \; f(\mathbf{r}, t) \; \vec{J}
\end{equation}

where

- $j = \sqrt{-1}$,
- $f(\mathbf{r}, t)$ is a time- and spatially-varying coefficient function, and
- $\vec{J}$ is the complex source vector field.

Note that $\vec{J}$ is provided via vector-valued functions, using the
[!param](/Kernels/VectorCurrentSource/source_real) and [!param](/Kernels/VectorCurrentSource/source_imag)
parameters for the real and imaginary components respectively.

## Example Input File Syntax

!listing vector_current_source.i block=Kernels/current_real

!syntax parameters /Kernels/VectorCurrentSource

!syntax inputs /Kernels/VectorCurrentSource

!syntax children /Kernels/VectorCurrentSource
