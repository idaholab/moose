# PorousPrimitiveVarMaterial

!syntax description /Materials/PorousPrimitiveVarMaterial

## Overview

This object uses the variable set

\begin{equation}
\begin{bmatrix}
p\\
\epsilon u\\
\epsilon v\\
\epsilon w\\
T_f
\end{bmatrix}
\end{equation}

where $\epsilon$ is the porosity, $u$, $v$, and $w$ are
the component velocities, $p$ is the pressure, and $T_f$ is the fluid
temperature.`PorousPrimitiveVarMaterial` takes these variables and computes all the
necessary quantities for solving the compressible porous version of the Euler
equations.

!syntax parameters /Materials/PorousPrimitiveVarMaterial

!syntax inputs /Materials/PorousPrimitiveVarMaterial

!syntax children /Materials/PorousPrimitiveVarMaterial
