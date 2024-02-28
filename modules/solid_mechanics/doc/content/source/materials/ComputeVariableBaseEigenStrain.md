# Compute Variable Base EigenStrain


!syntax description /Materials/ComputeVariableBaseEigenStrain

## Description

The material `ComputeVariableBaseEigenStrain` calculates a Rank-2 tensor eigenstrain
as a function of a Rank-2 tensor base and a scalar material property.
\begin{equation}
  \label{eqn:variable_base_eigenstrain}
  \boldsymbol{\epsilon}_{eigen} = p \boldsymbol{T} + \boldsymbol{A}
\end{equation}
where $\boldsymbol{\epsilon}_{eigen}$ is the calculated eigenstrain,
$p$ is a scalar material property, $\boldsymbol{T}$ is the tensor selected by
the user as the base of the eigenstrain, and $\boldsymbol{A}$ is the offset, or
constant initial, eigenstrain tensor.
The material property $p$ is used to introduce dependence of the eigenstrain on
the user-specified variable.

## Example Input File

!listing modules/combined/test/tests/DiffuseCreep/variable_base_eigen_strain.i block=Materials/eigenstrain

where the argument for the `base_tensor_property_name` parameter in the eigenstrain
is the same as the property parameter `gb_tensor_prop_name` argument as shown

!listing modules/combined/test/tests/DiffuseCreep/variable_base_eigen_strain.i block=Materials/aniso_tensor

and the argument for the `prefactor` parameter in the eigenstrain material matches
the function name (`f_name` parameter) in the [DerivativeParsedMaterial](/DerivativeParsedMaterial.md)

!listing modules/combined/test/tests/DiffuseCreep/variable_base_eigen_strain.i block=Materials/eigenstrain_prefactor

Finally, the `eigenstrain_name` parameter value must also be set for the strain calculator, and an example parameter setting is shown below:

!listing modules/combined/test/tests/DiffuseCreep/variable_base_eigen_strain.i block=Materials/strain

!syntax parameters /Materials/ComputeVariableBaseEigenStrain

!syntax inputs /Materials/ComputeVariableBaseEigenStrain

!syntax children /Materials/ComputeVariableBaseEigenStrain

!bibtex bibliography
