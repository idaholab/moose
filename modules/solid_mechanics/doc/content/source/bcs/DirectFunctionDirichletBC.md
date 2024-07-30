# DirectFunctionDirichletBC

!syntax description /BCs/DirectFunctionDirichletBC

## Overview

This applies a function Dirichlet BC meant to be used in conjunction with [CentralDifferenceDirect](source/timeintegrators/CentralDifferenceDirect.md).

BC's are applied by calculating the residual force needed to enforce BC's during a central difference solution update.

At each boundary node:

\begin{equation}
    F_g = \frac{u_g-u_n}{\Delta t^2} - \frac{v_{n-\frac{1}{2}}}{\Delta t}
\end{equation}

!syntax parameters /BCs/DirectFunctionDirichletBC

!syntax inputs /BCs/DirectFunctionDirichletBC

!syntax children /BCs/DirectFunctionDirichletBC
