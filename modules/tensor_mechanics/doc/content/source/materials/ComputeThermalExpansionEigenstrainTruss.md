# Compute Thermal Expansion Eigenstrain Truss

!syntax description /Materials/ComputeThermalExpansionEigenstrainTruss

## Description

`ComputeThemalExpansionEigenstrainTruss` calculates the thermal strain due to a change in temperature (from the stress-free temperature) using a constant thermal expansion coefficient. This thermal strain is applied only along the axial direction of the truss.

## Example Input Syntax

!listing modules/tensor_mechanics/test/tests/truss/truss_thermal.i block=Materials/thermal

!syntax parameters /Materials/ComputeThermalExpansionEigenstrainTruss

!syntax inputs /Materials/ComputeThermalExpansionEigenstrainTruss

!syntax children /Materials/ComputeThermalExpansionEigenstrainTruss
