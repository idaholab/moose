#ComputeThermalExpansionEigenstrain

!description /Materials/ComputeThermalExpansionEigenstrain
## Description

This model computes the eigenstrain tensor resulting from thermal expansion with a constant user-supplied thermal expansion coefficient, $\alpha$.  The current value of the thermal eigenstrain tensor, $\boldsymbol{\epsilon}^{th}$ can be computed at a given time as:

$$
\boldsymbol{\epsilon}^{th} = \alpha (T-T_{sf}) \boldsymbol{I} 
$$

where $T$ is the current temperature, $T_{sf}$ is the stress-free temperature, and $\boldsymbol{I}$ is the identity matrix.

For materials whose coefficient of thermal expansion varies as a function of temperature, see the [ComputeMeanThermalExpansionFunctionEigenstrain](ComputeMeanThermalExpansionFunctionEigenstrain.md) and [ComputeInstantaneousThermalExpansionFunctionEigenstrain](ComputeInstantaneousThermalExpansionFunctionEigenstrain.md) models.

!parameters /Materials/ComputeThermalExpansionEigenstrain

!inputfiles /Materials/ComputeThermalExpansionEigenstrain

!childobjects /Materials/ComputeThermalExpansionEigenstrain
