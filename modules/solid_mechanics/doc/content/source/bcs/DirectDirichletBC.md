# DirectDirichletBC

!syntax description /BCs/DirectDirichletBC

## Overview

This applies a Dirichlet BC meant to be used in conjunction with [CentralDifferenceDirect](source/timeintegrators/CentralDifferenceDirect.md).

BC's are applied by calculating the residual force needed,$\mathbf{F}_g$, to enforce BC's during a central difference solution update.

At each boundary node:

\begin{equation}
    F_g = \frac{u_g-u_n}{\Delta t^2} - \frac{v_{n-\frac{1}{2}}}{\Delta t}
\end{equation}

!syntax parameters /BCs/DirectDirichletBC

!syntax inputs /BCs/DirectDirichletBC

!syntax children /BCs/DirectDirichletBC
