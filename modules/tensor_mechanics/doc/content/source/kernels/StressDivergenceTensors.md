# Stress Divergence Tensors

!syntax description /Kernels/StressDivergenceTensors

## Description

The `StressDivergenceTensors` kernel calculates the residual of the stress divergence for 1D, 2D, and
3D problems in the Cartesian coordinate system.  This kernel can be automatically created with the
[TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md). Use of the tensor
mechanics master action is recommended to ensure the consistent setting of the _use_displaced_mesh_
parameter for the strain formulation selected.  For a detailed explanation of the settings for
_use_displaced_mesh_ in mechanics problems and the TensorMechanics Master Action usage, see the
[Introduction/StressDivergence](auto::/introduction/StressDivergence) page.

## Residual Calculation

!include modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

## Use with Planar Models

When used with 2D planar models (plane stres, plane strain, or generalized plane strain),
it is used to compute the residuals for the in-plane response. In all of these cases,
it assumed that the out-of-plane thickness is 1, and the computation of the in-plane
residuals is identical to that for the 3D case.

The only exception to this is the plane stress case with finite deformation, because
the out-of-plane thickness change can be significant, and in general is not spatially
uniform, so local thickness changes must be accounted for. In this case, the standard
residual is multiplied by the modified thickness, $t$, which is computed from the logarithmic
out of plane strain $\varepsilon_{oop}$ as:
\begin{equation}
t = e^{\varepsilon_{oop}}
\end{equation}
This correction is made for 2D planar models when the deformed mesh is used by setting
`use_displaced_mesh = true` and `out_of_plane_strain` is specified.

## Example Input File syntax

The Cartesian `StressDivergenceTensors` is the default case for the tensor
mechanics master action

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i
         block=Modules

Either 1, 2, or 3 displacement variables can be used in the stress divergence calculator for the
Cartesian system.

!syntax parameters /Kernels/StressDivergenceTensors

!include modules/tensor_mechanics/common/seealsoADStressDivergenceKernels.md

!syntax inputs /Kernels/StressDivergenceTensors

!syntax children /Kernels/StressDivergenceTensors
