# ADStressDivergenceRZTensors

!syntax description /Kernels/ADStressDivergenceRZTensors

## Description

The kernel `ADStressDivergenceRZTensors` solves the stress divergence equation
for an Axisymmetric problem in the cylindrical coordinate system on a 2D mesh.
Forward mode automatic differentiation is used to compute an exact Jacobian.

!alert warning title=Symmetry Assumed About the $z$-axis
The axis of symmetry must lie along the $z$-axis in a $\left(r, z, \theta \right)$
or cylindrical coordinate system. This symmetry orientation is required for the
calculation of the residual and of the jacobian, as defined in [eq-tensor_mechanics-RZ-stress-divergence].

## Residual Calculation

!include modules/tensor_mechanics/common/supplementalADStressDivergenceKernels.md

In cylindrical coordinates, the [divergence of a rank-2 tensor](https://en.wikipedia.org/wiki/Tensor_derivative_%28continuum_mechanics%29#Cylindrical_polar_coordinates_2)
includes mixed term contributions.  In the axisymmetric model we assume
symmetric loading conditions, in addition to the zero out-of-plane shear
strains, so that the residual computation is simplified.

\begin{equation}
  \label{eq-tensor_mechanics-RZ-stress-divergence}
  \begin{aligned}
  \nabla \sigma  & = \left[ \frac{\partial \sigma_{rr}}{\partial r} + \frac{u_r}{X_r}\sigma_{\theta \theta} + \frac{\partial \sigma_{rz}}{\partial z} \right] \hat{e}_r \\
   & + \left[ \frac{\partial \sigma_{zz}}{\partial z} + \frac{\partial \sigma_{rz}}{\partial r}    \right] \hat{e}_z
  \end{aligned}
\end{equation}

!alert note title=Notation Order Change
The axisymmetric system changes the order of the displacement vector from $(u_r, u_{\theta}, u_z)$,
usually seen in textbooks, to $(u_r, u_z, u_{\theta})$. Take care to follow this convention in your
input files and when adding extra stresses.

!alert note title=Use `RZ` Coordinate Type
The coordinate type in the `[Problem]` block of the input file must be set to
`coord_type = RZ`.

!syntax parameters /Kernels/ADStressDivergenceRZTensors

!include modules/tensor_mechanics/common/seealsoADStressDivergenceKernels.md

!syntax inputs /Kernels/ADStressDivergenceRZTensors

!syntax children /Kernels/ADStressDivergenceRZTensors
