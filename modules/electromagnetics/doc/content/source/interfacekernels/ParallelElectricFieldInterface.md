# ParallelElectricFieldInterface

!syntax description /InterfaceKernels/ParallelElectricFieldInterface

## Overview

!style halign=left
ParallelElectricFieldInterface is a vector InterfaceKernel object that
implements the condition

\begin{equation}
  \vec{E}_{1}^{\parallel} - \vec{E}_{2}^{\parallel} =0
\end{equation}

where

- $\vec{E}_{1}^{\parallel}$ is the parallel component of the electric field vector on the primary side of the interface, and
- $\vec{E}_{2}^{\parallel}$ is the parallel component of the electric field vector on the secondary side of the interface.

## Example Input File Syntax

!listing combined_props.i block=InterfaceKernels/parallel

!syntax parameters /InterfaceKernels/ParallelElectricFieldInterface

!syntax inputs /InterfaceKernels/ParallelElectricFieldInterface

!syntax children /InterfaceKernels/ParallelElectricFieldInterface
