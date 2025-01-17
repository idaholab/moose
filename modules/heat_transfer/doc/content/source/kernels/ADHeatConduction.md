# ADHeatConduction

## Description

`ADHeatConduction` is the implementation of [HeatConduction](/HeatConduction.md) within the framework of [!ac](AD). Please see the [HeatConduction](/HeatConduction.md) documentation for more information.

## Example Input File Syntax

The case demonstrates the use of `ADHeatConduction` where the
diffusion coefficient (thermal conductivity) is defined by an [ADGenericConstantMaterial](GenericConstantMaterial.md).

!listing modules/heat_transfer/test/tests/radiative_bcs/ad_radiative_bc_cyl.i
  start=Kernels
  end=Preconditioning
  remove=BCs

!syntax description /Kernels/ADHeatConduction

!syntax parameters /Kernels/ADHeatConduction

!syntax inputs /Kernels/ADHeatConduction

!syntax children /Kernels/ADHeatConduction

!bibtex bibliography
