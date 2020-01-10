# ComputeDilatationThermalExpansionFunctionEigenstrain

!syntax description /Materials/ComputeDilatationThermalExpansionFunctionEigenstrain

## Description

This model computes the eigenstrain tensor resulting from isotropic thermal expansion where the
temperature-dependent thermal expansion is defined by a user-supplied function that describes the
total dilatation as a function of temperature, $T$.

The thermal strain is given by,
\begin{equation}
  \boldsymbol{\epsilon}^{th} = \lambda (T) - \lambda(T_0) \boldsymbol{I}
  \label{eq:epsilon}
\end{equation}
where $T_0$ is the stress free temperature, $\lambda$ is the function that describes dilatation as a
function of temperature, and $\boldsymbol{I}$ is the identity matrix. For this material model,
$\lambda$ must be provided as a function.

!alert warning
Functions are not able to handle dual numbers at this time, so no automatic differentiation
information can be retained via the $\lambda$ function

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/thermal_expansion_function/dilatation.i
         block=Materials/thermal_expansion_strain

The `eigenstrain_name` parameter value must also be set for the strain calculator, and an example
parameter setting is shown below:

!listing modules/tensor_mechanics/test/tests/thermal_expansion_function/dilatation.i
         block=Modules/TensorMechanics/Master

!syntax parameters /Materials/ComputeDilatationThermalExpansionFunctionEigenstrain

!syntax inputs /Materials/ComputeDilatationThermalExpansionFunctionEigenstrain

!syntax children /Materials/ComputeDilatationThermalExpansionFunctionEigenstrain
