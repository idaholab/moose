# FVDivergence

!syntax description /FVKernels/FVDivergence

## Overview

The integral of the divergence operator acting on a vector field $\vec{u}$ in a finite volume
setting can be approximated as follows:

!equation
\int_{element} \nabla \cdot \vec{u} \approx \sum_\text{elem faces f} \vec{u}_f \cdot \vec{n_f} area_f

where $\vec{n_f}$ is the surface normal on each side of the element considered.

!syntax parameters /FVKernels/FVDivergence

!syntax inputs /FVKernels/FVDivergence

!syntax children /FVKernels/FVDivergence
