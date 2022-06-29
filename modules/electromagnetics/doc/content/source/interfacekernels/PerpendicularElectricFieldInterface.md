# PerpendicularElectricFieldInterface

!syntax description /InterfaceKernels/PerpendicularElectricFieldInterface

## Overview

!style halign=left
PerpendicularElectricFieldInterface is a vector InterfaceKernel object that
implements the condition

\begin{equation}
  \epsilon_1 \vec{E}_{1}^{\perp} - \epsilon_2 \vec{E}_{2}^{\perp} = \sigma_f
\end{equation}

where

- $\epsilon_1$ is the electric permittivity on the primary side of the interface,
- $\epsilon_2$ is the electric permittivity on the secondary side of the interface,
- $\vec{E}_{1}^{\perp}$ is the perpendicular component of the electric field vector on the primary side of the interface,
- $\vec{E}_{2}^{\perp}$ is the perpendicular component of the electric field vector on the secondary side of the interface, and
- $\sigma_f$ is the free electric charge on the interface.

## Example Input File Syntax

!listing combined_props.i block=InterfaceKernels/perpendicular


!syntax parameters /InterfaceKernels/PerpendicularElectricFieldInterface

!syntax inputs /InterfaceKernels/PerpendicularElectricFieldInterface

!syntax children /InterfaceKernels/PerpendicularElectricFieldInterface
