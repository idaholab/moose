# ConservedVarValuesMaterial

!syntax description /Materials/ConservedVarValuesMaterial

## Overview

This object takes a conserved free-flow variable set and computes all the
necessary quantities for solving the compressible free-flow Euler equations. The
conserved variable set in this case is

\begin{equation}
\begin{bmatrix}
\rho\\
\rho u\\
\rho v\\
\rho w\\
\rho e_t
\end{bmatrix}
\end{equation}

where $\rho$ is the density, $u$, $v$, and $w$ are the component velocities, and
$e_t$ is the total specific energy equivalent to $e + \left(\bm{a}\cdot\bm{a}\right)/2$ where
$e$ is the specific internal energy and $\bm{a} = \lbrace u, v, w\rbrace$.

!syntax parameters /Materials/ConservedVarValuesMaterial

!syntax inputs /Materials/ConservedVarValuesMaterial

!syntax children /Materials/ConservedVarValuesMaterial
