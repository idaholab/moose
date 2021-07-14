# PorousMixedVarMaterial

!syntax description /Materials/PorousMixedVarMaterial

## Overview

This object uses the variable set

\begin{equation}
\begin{bmatrix}
p\\
\epsilon \rho u\\
\epsilon \rho v\\
\epsilon \rho w\\
T_f
\end{bmatrix}
\end{equation}

where $\epsilon$ is the porosity, $\rho$ is the density, $u$, $v$, and $w$ are
the component velocities, $p$ is the pressure, and $T_f$ is the fluid
temperature.`PorousMixedVarMaterial` takes these variables and computes all the
necessary quantities for solving the compressible porous version of the Euler
equations.

!syntax parameters /Materials/PorousMixedVarMaterial

!syntax inputs /Materials/PorousMixedVarMaterial

!syntax children /Materials/PorousMixedVarMaterial
