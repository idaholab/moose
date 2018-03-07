# ComputeBeamThermalExpansionEigenstrain

!syntax description /Materials/ComputeBeamThermalExpansionEigenstrain

## Description
`ComputeBeamThemalExpansionEigenstrain` calculates the thermal strain due to a change in temperature (from the stress-free temperature) using a constant thermal expansion coefficient. This thermal strain is applied only along the axial direction of the beam.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/beam/thermal_expansion_small.i block=Materials/thermal

!syntax parameters /Materials/ComputeBeamThermalExpansionEigenstrain

!syntax inputs /Materials/ComputeBeamThermalExpansionEigenstrain

!syntax children /Materials/ComputeBeamThermalExpansionEigenstrain
