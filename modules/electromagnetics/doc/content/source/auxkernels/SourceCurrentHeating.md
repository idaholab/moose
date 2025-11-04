# SourceCurrentHeating

!syntax description /AuxKernels/SourceCurrentHeating

## Overview

!style halign=left
The SourceCurrentHeating object calculates the heating term imparted to the medium based on a supplied volumetric current. The term is defined as:

\begin{equation}
  0.5 * Re \left(\vec{J} \cdot \vec{E}^{*} \right)
\end{equation}

where

- $\vec{J}$ is the complex source vector field, and
- $\vec{E}^{*}$ is the complex conjugate of the electric field.

Note that $\vec{J}$ is provided via vector-valued functions, using the
[!param](/Kernels/VectorCurrentSource/source_real) and [!param](/Kernels/VectorCurrentSource/source_imag)
parameters for the real and imaginary components, respectively.

## Example Input File Syntax

!listing aux_current_source_heating.i block=AuxKernels/aux_current_heating

!syntax parameters /AuxKernels/SourceCurrentHeating

!syntax inputs /AuxKernels/SourceCurrentHeating

!syntax children /AuxKernels/SourceCurrentHeating
