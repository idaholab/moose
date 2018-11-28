# Compute Variable Eigenstrain

!syntax description /Materials/ComputeVariableEigenstrain

## Description

`ComputeVariableEigenstrain` calculates the eigenstrain as a function of a specified
variable as well as the contributions of the eigenstrain to the first and second
order derivatives of the elastic strain.
This class is most often only used in phase field simulations where first and
second derivatives are required and the limitation on elastic only strains is
not overly restrictive.

The Rank-2 tensor eigenstrain is calculated as a function of a Rank-2 tensor base
and a scalar material property.
\begin{equation}
  \label{eqn:variable_base_eigenstrain}
  \boldsymbol{\epsilon} = p \boldsymbol{T}
\end{equation}
where $\boldsymbol{\epsilon}_{eigen}$ is the computed eigenstrain,
$p$ is a scalar material property, and $\boldsymbol{T}$ is the tensor selected by
the user as the base of the eigenstrain.
The material property $p$ is used to introduce dependence of the eigenstrain on
the user-specified variable.

The contributions of the eigenstrain to the first and second elastic strain
derivatives are calculated with use of the MOOSE
[DerivativeMaterialInterface](materials/DerivativeMaterialInterface.md)
applied to the prefactor variables.
\begin{equation}
  \label{eqn:derivatives}
  \begin{aligned}
  \nabla \cdot \boldsymbol{\epsilon} & = \left( \nabla \cdot p \right) \boldsymbol{T} \\
  \nabla^2 \cdot \boldsymbol{\epsilon} & = \left( \nabla^2 \cdot p \right) \boldsymbol{T}
  \end{aligned}
\end{equation}
where $\nabla \cdot \boldsymbol{\epsilon}$ and $\nabla^2 \cdot \boldsymbol{\epsilon}$ are
the first and second derivatives of the elastic strain contributions due to the
eigenstrain.

!alert warning title=Use with Elastic Strain Only
This class assumes the presence of only elastic strain in the computation of the
first and second derivatives.

## Example Input File

!listing modules/combined/test/tests/multiphase_mechanics/simpleeigenstrain.i block=Materials/eigenstrain

where the argument for the `args` parameter in the eigenstrain matches the name
of the coupled variable, here shown as an auxvariable

!listing modules/combined/test/tests/multiphase_mechanics/simpleeigenstrain.i block=AuxVariables/c

and the argument for the `prefactor` parameter in the eigenstrain material matches
the function name (`f_name` parameter) in the [DerivativeParsedMaterial](/DerivativeParsedMaterial.md)

!listing modules/combined/test/tests/multiphase_mechanics/simpleeigenstrain.i block=Materials/prefactor

Finally, the `eigenstrain_name` parameter value must also be set for the strain calculator, and an example parameter setting is shown below:

!listing modules/combined/test/tests/multiphase_mechanics/simpleeigenstrain.i block=Materials/strain

!syntax parameters /Materials/ComputeVariableEigenstrain

!syntax inputs /Materials/ComputeVariableEigenstrain

!syntax children /Materials/ComputeVariableEigenstrain

!bibtex bibliography
