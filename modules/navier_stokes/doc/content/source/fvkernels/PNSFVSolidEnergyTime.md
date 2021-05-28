# PNSFVSolidEnergyTime

!syntax description /FVKernels/PNSFVSolidEnergyTime

## Overview

This object implements the residual

\begin{equation}
\left(1 - \epsilon\right) \rho_s c_{p,s} \frac{\partial u}{\partial t}
\end{equation}

where $\epsilon$ is the porosity, $rho_s$ is the solid density, and $c_{p,s}$ is
the specific heat capacity of the solid.

!syntax parameters /FVKernels/PNSFVSolidEnergyTime

!syntax inputs /FVKernels/PNSFVSolidEnergyTime

!syntax children /FVKernels/PNSFVSolidEnergyTime
