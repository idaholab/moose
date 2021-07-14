# FVOrthogonalBoundaryDiffusion

!syntax description /FVBCs/FVOrthogonalBoundaryDiffusion

## Overview

This object implements a boundary flux equivalent to

\begin{equation}
-D \frac{u_b - u_C}{\vert \bm{d}_{Cf}\vert}
\end{equation}

where $D$ is a diffusion coefficient, $u_b$ is a boundary value computed through
the provided function parameter `function`, $u_C$ is the value of the diffusing
quantity at the neighboring cell centroid $C$, and $\bm{d}_{Cf}$ is the distance
vector drawn from the cell centroid to the boundary face centroid.

!syntax parameters /FVBCs/FVOrthogonalBoundaryDiffusion

!syntax inputs /FVBCs/FVOrthogonalBoundaryDiffusion

!syntax children /FVBCs/FVOrthogonalBoundaryDiffusion
