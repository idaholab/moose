# ComputeBeamEigenstrainFromAuxVar

!syntax description /Materials/ComputeBeamEigenstrainFromAuxVar

## Description
`ComputeBeamEigenstainFromAuxVar` class takes in a set of 3 displacement or rotation auxvariables or both and uses them to calculate the displacement and rotational beam eigenstrain in the global coordinate system. If either displacement or rotational aux variables are provided, they should be provided in all three global coordinate directions.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/beam/eigenstrain/eigenstrain_from_aux_var.i block=Materials/thermal

!syntax parameters /Materials/ComputeBeamEigenstrainFromAuxVar

!syntax inputs /Materials/ComputeBeamEigenstrainFromAuxVar

!syntax children /Materials/ComputeBeamEigenstrainFromAuxVar
