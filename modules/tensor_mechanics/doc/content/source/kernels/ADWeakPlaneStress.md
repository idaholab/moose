# ADWeakPlaneStress

!syntax description /Kernels/ADWeakPlaneStress

## Description

In 2D plane stress conditions, the out-of-plane stress is zero.  The `ADWeakPlaneStress` kernel
operates on an out-of-plane strain variable and computes the following residual:
\begin{equation}
  \int \phi \; \sigma_{zz} \; \textrm{dV}.
\end{equation}
Thus, the out-of-plane stress is driven toward zero but may not be strictly zero everywhere.
The computed out-of-plane strain may vary at different points on the plane.

For finite deformation models, this kernel should be run on the displaced mesh by setting
`use_displaced_mesh = true`.

!syntax parameters /Kernels/ADWeakPlaneStress

!syntax inputs /Kernels/ADWeakPlaneStress

!syntax children /Kernels/ADWeakPlaneStress

!bibtex bibliography
