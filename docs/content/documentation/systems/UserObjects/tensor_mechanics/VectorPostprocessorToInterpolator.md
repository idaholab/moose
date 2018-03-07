# VectorPostprocessorToInterpolator

!syntax description /UserObjects/VectorPostprocessorToInterpolator

## Description
`VectorPostprocessorToInterpolator` reads all vector postprocessor csv files with a given pattern. The position vector in the `from_component` direction is extracted from the csv files along with data corresponding to a list of variables. The time vector is either read in from a seperate `time_file` or is calculated using a constant `time_step`. With the position, time and variable values, a bilinear interpolation object is constructed for each variable. This interpolation object can be used by other classes such as [ComputeBeamEigenstrainFromVPP](/ComputeBeamEigenstrainFromVPP,md) to calculate the variable values at a given quadrature point at a given time. 

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/beam/eigenstrain/eigenstrain_from_vpp.i block=UserObjects/disp_uo

!syntax parameters /UserObjects/VectorPostprocessorToInterpolator

!syntax inputs /UserObjects/VectorPostprocessorToInterpolator

!syntax children /UserObjects/VectorPostprocessorToInterpolator
