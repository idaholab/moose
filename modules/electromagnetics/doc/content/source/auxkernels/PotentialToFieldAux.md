# PotentialToFieldAux

!syntax description /AuxKernels/PotentialToFieldAux

## Overview

This AuxKernel object uses the following equation for electric field:

\begin{equation}
  \mathbf{E} = s \nabla V
\end{equation}

where

- $\mathbf{E}$ is the electric field in units of Volts per meter,
- $V$ is the electrostatic potential in units of Volts, and
- $s$ is a coefficient set via the [!param](/AuxKernels/PotentialToFieldAux/sign) 
  parameter, and defaults to $-1$.

The sign of the resulting calculation can be changed from the default, if needed,
by setting [!param](/AuxKernels/PotentialToFieldAux/sign) to `positive`. This option 
is the result of the desire to keep the object code as general as possible for use in 
other scenarios where the positive gradient of a solution variable is desired, while 
still retaining the traditional and expected electrostatic field result from various 
electrodynamics texts by default.

## Example Input File Syntax

!listing eigen_base.i block=AuxKernels/Ex_aux

!syntax parameters /AuxKernels/PotentialToFieldAux

!syntax inputs /AuxKernels/PotentialToFieldAux

!syntax children /AuxKernels/PotentialToFieldAux
