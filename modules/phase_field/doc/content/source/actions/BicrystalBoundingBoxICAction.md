# BicrystalBoundingBoxICAction

!syntax description /ICs/PolycrystalICs/BicrystalBoundingBoxIC/BicrystalBoundingBoxICAction

## Overview

This action creates the initial conditions (ICs) for the two order parameters used to represent a bicrystal. It creates a grain structure with the first grain represented by a rectangle (2D) or rectangular prism (3D) embedded in the second grain. One common use is to create a bicrystal with a single straight grain boundary between the two grains.

The action creates two ICs with the [BoundingBoxIC](/BoundingBoxIC.md), one for each order parameter. Each are defined with the same bounding box coordinates, but for the first order parameter `inside = 1.0` and `outside = 0.0` and for the second `inside = 0.0` and `outside = 1.0`.

## Example Input File Syntax

The `BicrystalBoundingBoxICAction` is accessed through the `ICs/PolycrystalICs` block, as shown below.

!listing modules/phase_field/test/tests/grain_growth/boundingbox.i block=ICs

!syntax parameters /ICs/PolycrystalICs/BicrystalBoundingBoxIC/BicrystalBoundingBoxICAction

!syntax children /ICs/PolycrystalICs/BicrystalBoundingBoxIC/BicrystalBoundingBoxICAction
