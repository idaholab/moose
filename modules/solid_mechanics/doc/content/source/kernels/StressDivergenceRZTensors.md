# Stress Divergence RZ Tensors

!syntax description /Kernels/StressDivergenceRZTensors

## Description

The kernel `StressDivergenceRZTensors` solves the stress divergence equation for an Axisymmetric
problem in the cylindrical coordinate system on a 2D mesh.

!alert warning title=Symmetry Assumed About the $z$-axis
The axis of symmetry must lie along the $z$-axis in a $\left(r, z, \theta \right)$
or cylindrical coordinate system. This symmetry orientation is required for the
calculation of the residual and of the jacobian, as defined in [eq-tensor_mechanics-RZ-stress-divergence].

The `StressDivergenceRZTensors` kernel can be automatically created with the
[TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md). Use of the tensor
mechanics master action is recommended to ensure the consistent setting of the `use_displaced_mesh`
parameter for the strain formulation selected.  For a detailed explanation of the settings for
_use_displaced_mesh_ in mechanics problems and the TensorMechanics Master Action usage, see the
[Introduction/Stress Divergence](/tensor_mechanics/StressDivergence.md) page.


## Residual Calculation

!include modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

In cylindrical coordinates, the
[divergence of a rank-2 tensor](https://en.wikipedia.org/wiki/Tensor_derivative_%28continuum_mechanics%29#Cylindrical_polar_coordinates_2)
includes mixed term contributions.  In the axisymmetric model we assume symmetric loading conditions,
in addition to the zero out-of-plane shear strains, so that the residual computation is simplified.

\begin{equation}
  \label{eq-tensor_mechanics-RZ-stress-divergence}
  \begin{aligned}
  \nabla \sigma  & = \left[ \frac{\partial \sigma_{rr}}{\partial r} + \frac{u_r}{X_r}\sigma_{\theta \theta} + \frac{\partial \sigma_{rz}}{\partial z} \right] \hat{e}_r \\
   & + \left[ \frac{\partial \sigma_{zz}}{\partial z} + \frac{\partial \sigma_{rz}}{\partial r}    \right] \hat{e}_z
  \end{aligned}
\end{equation}

The calculation of the Jacobian is similarly complex, requiring up to four terms in the calculation
of the diagonal entries.

!alert note title=Notation Order Change
The axisymmetric system changes the order of the displacement vector from $(u_r, u_{\theta}, u_z)$,
usually seen in textbooks, to $(u_r, u_z, u_{\theta})$. Take care to follow this convention in your
input files and when adding extra stresses.

## Example Input File

!alert note title=Use RZ Coordinate Type
The coordinate type in the Problem block of the input file must be set to
+`COORD_TYPE = RZ`+.

Using the tensor mechanics master action, as shown

!listing modules/tensor_mechanics/test/tests/2D_geometries/2D-RZ_finiteStrain_test.i block=Modules/TensorMechanics/Master

the `StressDivergenceRZTensors` kernel will be automatically built when the coordinate system in the
Problem block is specified for the axisymmetric RZ system,

!listing modules/tensor_mechanics/test/tests/2D_geometries/2D-RZ_finiteStrain_test.i block=Problem

and only two displacement variables are provided:

!listing modules/tensor_mechanics/test/tests/2D_geometries/2D-RZ_finiteStrain_test.i block=GlobalParams

!syntax parameters /Kernels/StressDivergenceRZTensors

!include modules/tensor_mechanics/common/seealsoADStressDivergenceKernels.md

!syntax inputs /Kernels/StressDivergenceRZTensors

!syntax children /Kernels/StressDivergenceRZTensors
