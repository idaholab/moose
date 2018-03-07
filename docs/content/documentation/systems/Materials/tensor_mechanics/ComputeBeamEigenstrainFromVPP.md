# ComputeBeamEigenstrainFromVPP

!syntax description /Materials/ComputeBeamEigenstrainFromVPP

## Description
`ComputeBeamEigenstrainFromVPP` computes the displacement and rotational eigenstrain from the corresponding vector postprocessor csv files. [VectorPostprocessorToInterpolater](/VectorPostprocessorToInterpolater.h) userobject first reads the csv files and creates a bilinear intepolation object as a function of position and time for each variable. For each quadrature point and time, the interpolated values of the variable (using the qp point position along `to_component` and time) are assigned to the displacement and rotational eigenstrain material properties.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/beam/eigenstrain/eigenstrain_from_vpp.i block=Materials/thermal

!syntax parameters /Materials/ComputeBeamEigenstrainFromVPP

!syntax inputs /Materials/ComputeBeamEigenstrainFromVPP

!syntax children /Materials/ComputeBeamEigenstrainFromVPP

