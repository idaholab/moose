# ADStressDivergenceTensors

!syntax description /Kernels/ADStressDivergenceTensors<RESIDUAL>

## Description

The `ADStressDivergenceTensors` kernel calculates the residual of the stress
divergence for 1D, 2D, and 3D problems in the Cartesian coordinate system.
Forward mode automatic differentiation is used to compute an exact Jacobian.

Either 1, 2, or 3 displacement variables can be used in the stress divergence
calculator for the Cartesian system.

## Residual Calculation

!include modules/tensor_mechanics/common/supplementalADStressDivergenceKernels.md

## Example Input File syntax

!syntax parameters /Kernels/ADStressDivergenceTensors<RESIDUAL>

!include modules/tensor_mechanics/common/seealsoADStressDivergenceKernels.md

!syntax inputs /Kernels/ADStressDivergenceTensors<RESIDUAL>

!syntax children /Kernels/ADStressDivergenceTensors<RESIDUAL>
