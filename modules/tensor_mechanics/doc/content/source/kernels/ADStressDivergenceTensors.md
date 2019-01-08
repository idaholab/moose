# Stress Divergence Tensors with Automatic Differentiation

!syntax description /ADKernels/ADStressDivergenceTensors<RESIDUAL>

## Description

The `ADStressDivergenceTensors` kernel calculates the residual of the stress
divergence for 1D, 2D, and 3D problems in the Cartesian coordinate system.

The Jacobian in `ADStressDivergenceTensors` is computed using forward automatic
differentiation.

## Residual Calculation

!include modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

Either 1, 2, or 3 displacement variables can be used in the stress divergence
calculator for the Cartesian system.

!syntax parameters /ADKernels/ADStressDivergenceTensors<RESIDUAL>

!syntax inputs /ADKernels/ADStressDivergenceTensors<RESIDUAL>

!syntax children /ADKernels/ADStressDivergenceTensors<RESIDUAL>
