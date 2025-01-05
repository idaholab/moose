# ADHeatConductionTimeDerivative

## Description

`ADHeatConductionTimeDerivative` is the implementation of [HeatConductionTimeDerivative](/HeatConductionTimeDerivative.md) within the framework of [!ac](AD). Please see the [HeatConductionTimeDerivative](/HeatConductionTimeDerivative.md) documentation for more information.

For this object, the diffusion coefficient can either be an `ADMaterial` or traditional `Material`, though the Jacobian will be perfect if using an `ADMaterial`.

## Example Input File Syntax

The case demonstrates the use of `ADHeatConductionTimeDerivative` where the
diffusion coefficient (thermal conductivity) is defined by an [ADGenericConstantMaterial](ADGenericConstantMaterial.md)

!listing modules/heat_transfer/test/tests/verify_against_analytical/ad_1D_transient.i
  start=Kernels
  end=Materials
  remove=BCs

!syntax parameters /Kernels/ADHeatConductionTimeDerivative

!syntax inputs /Kernels/ADHeatConductionTimeDerivative

!syntax children /Kernels/ADHeatConductionTimeDerivative
