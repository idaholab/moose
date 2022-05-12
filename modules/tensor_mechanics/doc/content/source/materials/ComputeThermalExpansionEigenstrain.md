# Compute Thermal Expansion Eigenstrain

!syntax description /Materials/ComputeThermalExpansionEigenstrain

## Description

This model computes the eigenstrain tensor resulting from isotropic thermal
expansion where the constant thermal expansion is defined by a user-supplied
scalar linear thermal-expansion coefficient, $\alpha$. The thermal expansion
eigenstrain is then computed as

\begin{equation}
\boldsymbol{\epsilon}^{thermal} = \alpha \cdot \left( T - T_{stress\_free} \right) \boldsymbol{I}
\end{equation}

where $T$ is the current temperature, $T_{stress\_free}$ is the stress free
temperature, and $\boldsymbol{I}$ is the identity matrix.

An automatic differentiation version of this object is available as `ADComputeThermalExpansionEigenstrain`.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/thermal_expansion/constant_expansion_stress_free_temp.i block=Materials/thermal_expansion_strain

The `eigenstrain_names` parameter value must also be set for the strain calculator, and an example parameter setting is shown below:

!listing modules/tensor_mechanics/test/tests/thermal_expansion/constant_expansion_stress_free_temp.i block=Modules/TensorMechanics/Master

!syntax parameters /Materials/ComputeThermalExpansionEigenstrain

!syntax inputs /Materials/ComputeThermalExpansionEigenstrain

!syntax children /Materials/ComputeThermalExpansionEigenstrain
