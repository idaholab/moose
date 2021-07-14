# FVOrthogonalDiffusion

!syntax description /FVKernels/FVOrthogonalDiffusion

## Overview

This object implements an intercell flux equivalent to

\begin{equation}
-D \frac{u_N - u_E}{\vert \bm{d}_{EN}\vert}
\end{equation}

where $D$ is a diffusion coefficient that is linearly interpolated between the
cell centroids $N$ and $E$ (denoting `neighbor` and `element` respectively),
$u_N$ is the value of the diffusion quantity at the `neighbor` cell centroid,
$u_E$ is the value of the diffusing quantity at the `element` cell
and $\bm{d}_{EN}$ is the distance vector drawn from the `element` centroid to
the `neighbor` centroid.

!syntax parameters /FVKernels/FVOrthogonalDiffusion

!syntax inputs /FVKernels/FVOrthogonalDiffusion

!syntax children /FVKernels/FVOrthogonalDiffusion
