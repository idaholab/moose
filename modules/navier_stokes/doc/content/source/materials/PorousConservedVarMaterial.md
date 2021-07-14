# PorousConservedVarMaterial

!syntax description /Materials/PorousConservedVarMaterial

## Overview

This object is the porous version of the free-flow object
[ConservedVarValuesMaterial.md]. It uses the variable set

\begin{equation}
\begin{bmatrix}
\rho\\
\epsilon \rho u\\
\epsilon \rho v\\
\epsilon \rho w\\
\rho e_t
\end{bmatrix}
\end{equation}

where $\epsilon$ is the porosity, $\rho$ is the density, $u$, $v$, and $w$ are the component velocities, and
$e_t$ is the total specific energy equivalent to $e + \left(\bm{a}\cdot\bm{a}\right)/2$ where
$e$ is the specific internal energy and $\bm{a} = \lbrace u, v,
w\rbrace$. `PorousConservedVarMaterial` takes these variables and computes all the
necessary quantities for solving the compressible porous version of the Euler equations.

!syntax parameters /Materials/PorousConservedVarMaterial

!syntax inputs /Materials/PorousConservedVarMaterial

!syntax children /Materials/PorousConservedVarMaterial
