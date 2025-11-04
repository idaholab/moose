# WeakPlaneStress

!syntax description /Kernels/WeakPlaneStress

## Description

In 2D plane stress conditions, the out-of-plane stress is zero.  The `WeakPlaneStress` kernel
operates on an out-of-plane strain variable and computes the following residual:
\begin{equation}
  \int \phi \; \sigma_{zz} \; \textrm{dV}.
\end{equation}
Thus, the out-of-plane stress is driven toward zero but may not be strictly zero everywhere.
The computed out-of-plane strain may vary at different points on the plane. This approach
for enforcing plane-stress conditions has the advantage that it allows the use of 3D
mechanical constitutive models for all cases, without requiring special implementations
of the models for 2D plane stress conditions [!cite](chen_mixed_2024).

For finite deformation models, this kernel should be run on the displaced mesh by setting
`use_displaced_mesh = true`.

!syntax parameters /Kernels/WeakPlaneStress

!syntax inputs /Kernels/WeakPlaneStress

!syntax children /Kernels/WeakPlaneStress

!bibtex bibliography
