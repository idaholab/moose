# ADComputeElongationThermalExpansionFunctionEigenstrain

!syntax description /Materials/ADComputeElongationThermalExpansionFunctionEigenstrain

## Description

This model computes the eigenstrain tensor resulting from isotropic thermal expansion where the
temperature-dependent thermal expansion is defined by a user-supplied function that describes the
total elongation as a function of temperature, $T$.

The thermal strain is given by,
\begin{equation}
  \boldsymbol{\epsilon}^{th} = \frac{\lambda (T) - \lambda(T_0)}{1+\lambda(T_0)}
  \label{eq:epsilon}
\end{equation}
where $T_0$ is the stress free temperature and $\lambda$ is the function that describes elongation
as a function of temperature. For this material model, $\lamda$ must be provided as a function.

Note that the denominator in this [eq:epsilon] is a correction to account for the ratio of
$L_{(T_{sf})}$ to $L_{(T_{ref})}$. As discussed in [!cite](niffenegger2012proper), that ratio is
very close to 1, so it is not strictly necessary to include that correction, but it is done here for
completeness.

!alert warning
Functions are not able to handle dual numbers at this time, so no automatic differentiation
information can be retained via the $\lambda$ function

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_thermal_expansion_function/elongation.i
         block=Materials/thermal_expansion_strain

The `eigenstrain_name` parameter value must also be set for the strain calculator, and an example
parameter setting is shown below:

!listing modules/tensor_mechanics/test/tests/ad_thermal_expansion_function/elongation.i
         block=Modules/TensorMechanics/Master

!syntax parameters /Materials/ADComputeElongationThermalExpansionFunctionEigenstrain

!syntax inputs /Materials/ADComputeElongationThermalExpansionFunctionEigenstrain

!syntax children /Materials/ADComputeElongationThermalExpansionFunctionEigenstrain
