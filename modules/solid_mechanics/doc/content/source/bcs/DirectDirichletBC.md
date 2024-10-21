# DirectDirichletBC

!syntax description /BCs/DirectDirichletBC

## Overview

This applies a Dirichlet BC meant to be used in conjunction with [DirectCentralDifference](source/timeintegrators/DirectCentralDifference.md).

BC's are applied by calculating the residual force needed,$\mathbf{F}_g$, to enforce BC's during a central difference solution update.

At each boundary node:

\begin{equation}
    \begin{aligned}
        F_g & = \frac{u_g-u_n}{\Delta t * \Delta t_\text{avg}} - \frac{v_{n-\frac{1}{2}}}{\Delta t_\text{avg}},\\
        \Delta t_\text{avg} & = \frac{\Delta t_n-\Delta t_{n-1}}{2}
    \end{aligned}
\end{equation}

where $\mathbf{F_g}$ is the force required to enforce the BC and ${\mathbf{u}_g}$ is the displacement to be enforced.

!syntax parameters /BCs/DirectDirichletBC

!syntax inputs /BCs/DirectDirichletBC

!syntax children /BCs/DirectDirichletBC
