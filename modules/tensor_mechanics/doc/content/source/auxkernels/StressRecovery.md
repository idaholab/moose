# Stress Recovery

!syntax description /AuxKernels/StressRecovery

## Description

This AuxKernel `StressRecovery` is derived from `NodalPatchRecovery` and implements the `computeValue` method by calculating stress at quadrature point.

This AuxKernel is intended for use to post process a smooth nodal stress field.
## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/stress_recovery/patch/patch.i block=AuxKernels/stress_xx_recovered
!listing modules/tensor_mechanics/test/tests/stress_recovery/patch/patch.i block=AuxKernels/stress_yy_recovered
!listing modules/tensor_mechanics/test/tests/stress_recovery/patch/stress_concentration.i block=AuxKernels/stress_xx_recovered
!listing modules/tensor_mechanics/test/tests/stress_recovery/patch/stress_concentration.i block=AuxKernels/stress_yy_recovered

An AuxVariable is required to store the `StressRecovery` AuxKernel information.

!syntax parameters /AuxKernels/StressRecovery

!syntax inputs /AuxKernels/StressRecovery

!syntax children /AuxKernels/StressRecovery
