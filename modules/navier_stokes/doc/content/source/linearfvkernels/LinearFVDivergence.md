# LinearFVDivergence

!syntax description /LinearFVKernels/LinearFVDivergence

## Overview

The element-wise integral of the divergence operator acting on a vector
field $\vec{u}$ in a finite volume setting can be approximated as follows:

!equation
\int\limits_{V_C} \nabla \cdot \vec{u} \approx \sum\limits_\text{f} \vec{u}_f \cdot \vec{n_f} |S_f|

where $\vec{u}_f$, $\vec{n_f}$ and $|S_f|$ are the approximated field value, normal and area of surface $f$.
Unlike [FVDivergence.md], this kernel expects the face flux
($\vec{u}_f \cdot \vec{n_f}$) as an input through the [!param](/LinearFVKernels/LinearFVDivergence/face_flux) parameter.

!syntax parameters /LinearFVKernels/LinearFVDivergence

!syntax inputs /LinearFVKernels/LinearFVDivergence

!syntax children /LinearFVKernels/LinearFVDivergence
