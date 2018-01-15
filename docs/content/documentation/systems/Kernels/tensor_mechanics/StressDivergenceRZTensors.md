#Stress Divergence RZ Tensors
!syntax description /Kernels/StressDivergenceRZTensors

##Description
The kernel `StressDivergenceRZTensors` solves the stress divergence equation for an Axisymmetric problem in the cylindrical coordinate system on a 2D mesh.

!!! info
    The `COORD_TYPE` in the Problem block of the input file must be set to RZ.


The `StressDivergenceRZTensors` kernel can be automatically created with the [TensorMechanics Master Action](/systems/Modules/TensorMechanics/Master/index.md). Use of the tensor mechanics master action is recommended to ensure the consistent setting of the _use_displaced_mesh_ parameter for the strain formulation selected.
For a detailed explanation of the settings for _use_displaced_mesh_ in mechanics problems and the TensorMechanics Master Action usage, see the [Introduction/StressDivergence](auto::/introduction/StressDivergence) page.

## Residual Calculation
!include docs/content/documentation/modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

As in the Stress Divergence kernel for Cartesian coordinates, [StressDivergenceTensors](/StressDivergenceTensors.md), the stress divergence kernel for the Axisymmetric simulations includes the stress divergence calculation for the residual and the calculation of the Jacobian matrix.

In cylindrical coordinates, the [divergence of a rank-2 tensor](https://en.wikipedia.org/wiki/Tensor_derivative_%28continuum_mechanics%29#Cylindrical_polar_coordinates_2) includes mixed term contributions.  In the axisymmetric model we assume symmetric loading conditions, in addition to the zero out-of-plane shear strains, so that the residual computation is simplified.

\begin{equation}
\begin{aligned}
\nabla \sigma  & = \left[ \frac{\partial \sigma_{rr}}{\partial r} + \frac{u_r}{X_r}\sigma_{\theta \theta} + \frac{\partial \sigma_{rz}}{\partial z} \right] \hat{e}_r \\
 & + \left[ \frac{\partial \sigma_{zz}}{\partial z} + \frac{\partial \sigma_{rz}}{\partial r}    \right] \hat{e}_z
\end{aligned}
\end{equation}

!!! note
    This calculation of the residual and the Jacobian calculation require the axis of symmetry lies along the $z$-axis.

The calculation of the Jacobian is similarly complex, requiring up to four terms in the calculation of the diagonal entries.

## Example Input File syntax
Using the tensor mechanics master action, as shown
!listing modules/tensor_mechanics/test/tests/2D_geometries/2D-RZ_finiteStrain_test.i block=Modules/TensorMechanics/Master

the `StressDivergenceRZTensors` kernel will be automatically built when the coordinate system in the Problem block is specified for the axisymmetric RZ system,
!listing modules/tensor_mechanics/test/tests/2D_geometries/2D-RZ_finiteStrain_test.i block=Problem

and only two displacement variables are provided:
!listing modules/tensor_mechanics/test/tests/2D_geometries/2D-RZ_finiteStrain_test.i block=GlobalParams

!syntax parameters /Kernels/StressDivergenceRZTensors

!syntax inputs /Kernels/StressDivergenceRZTensors

!syntax children /Kernels/StressDivergenceRZTensors
