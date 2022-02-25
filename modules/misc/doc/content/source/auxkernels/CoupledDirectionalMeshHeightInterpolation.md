# CoupledDirectionalMeshHeightInterpolation

!syntax description /AuxKernels/CoupledDirectionalMeshHeightInterpolation

## Description

This object scales a user-provied coupled variable (either a solution variable or an auxiliary variable) by its position relative to the maximum and minimum coordinate of the undeformed mesh in a user-specified direction. When the coordinate is equal to the minimum coordinate, the scaling factor is 0, while when it is at the maximum coordinate, the scaling factor is 1. For locations between these mesh bounds, the scaling factor is linearly interpolated between those values.

## Example Input File Syntax

!listing modules/misc/test/tests/coupled_directional_mesh_height_interpolation/coupled_directional_mesh_height_interpolation.i block=AuxKernels/interpolation

!syntax inputs /AuxKernels/CoupledDirectionalMeshHeightInterpolation

!syntax children /AuxKernels/CoupledDirectionalMeshHeightInterpolation
