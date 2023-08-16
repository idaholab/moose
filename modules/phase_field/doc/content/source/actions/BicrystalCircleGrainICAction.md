# BicrystalCircleGrainICAction

!syntax description /ICs/PolycrystalICs/BicrystalCircleGrainIC/BicrystalCircleGrainICAction

## Overview

This action creates the initial conditions (ICs) for the two order parameters used to represent a bicrystal. It creates a grain structure with the first grain represented by a circle (2D) or sphere (3D) embedded in the second grain.

The action creates two ICs with the [SmoothCircleIC](/SmoothCircleIC.md), one for each order parameter. Each are defined with the same center coordinates and radius, but for the first order parameter `invalue = 1.0` and `outvalue = 0.0` and for the second `invalue = 0.0` and `outvalue = 1.0`.

## Example Input File Syntax

The `BicrystalCircleGrainICAction` is accessed through the `ICs/PolycrystalICs` block, as shown below.

!listing modules/phase_field/test/tests/grain_growth/test.i block=ICs

!syntax parameters /ICs/PolycrystalICs/BicrystalCircleGrainIC/BicrystalCircleGrainICAction

!syntax children /ICs/PolycrystalICs/BicrystalCircleGrainIC/BicrystalCircleGrainICAction
