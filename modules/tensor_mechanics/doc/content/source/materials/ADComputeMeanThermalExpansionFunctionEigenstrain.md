# ADComputeMeanThermalExpansionFunctionEigenstrain

!syntax description /Materials/ADComputeMeanThermalExpansionFunctionEigenstrain

## Description

This model computes the eigenstrain tensor resulting from isotropic thermal expansion where the
temperature-dependent thermal expansion is defined by a user-supplied function that describes the
mean thermal expansion coefficient $\bar{\alpha}$ as a function of temperature, $T$. This function is
defined relative to a reference temperature, $T_{ref}$, such that the total expansion at a given
temperature relative to the refererence temperature is $\bar{\alpha}(T-T_{ref})$.  Following the
notation of [!cite](niffenegger2012proper), $\bar{\alpha}_{(T_{ref},T)}$ is defined as:

\begin{equation}
\bar{\alpha}_{(T_{ref},T)} = \frac{L_{(T)} - L_{(T_{ref})}}{L_{(T_{ref})}(T-T_{ref})}
\end{equation}
where $L_{T}$ is the length of a body at the current temperature, and $L_{T_{ref}}$ is the length of
that body at the reference temperature.

It is important to emphasize that this reference temperature is tied to the definition of the thermal
expansion function, and differs in general from the stress-free temperature for a specific
simulation.  For the general case where the stress-free temperature, $T_{sf}$, differs from the
reference temperature, the total thermal expansion eigenstrain is computed as:

\begin{equation}
\boldsymbol{\epsilon}^{th} = \frac{\bar{\alpha}_{(T_{ref},T)}(T-T_{ref}) - \bar{\alpha}_{(T_{ref},T_{sf})}(T_{sf}-T_{ref})}
{1 + \bar{\alpha}_{(T_{ref},T_{sf})}(T_{sf}-T_{ref})} \cdot \boldsymbol{I}
\end{equation}
where $T$ is the current temperature and $\boldsymbol{I}$ is the identity matrix.  Note that the
denominator in this equation is a correction to account for the ratio of $L_{(T_{sf})}$ to
$L_{(T_{ref})}$. As discussed in [!cite](niffenegger2012proper), that ratio is very close to 1, so it
is not strictly necessary to include that correction, but it is done here for completeness.

!alert warning
Functions are not able to handle dual numbers at this time, so no automatic differentiation
information can be retained via the $\bar{\alpha}$ function

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_thermal_expansion_function/finite_const.i
         block=Materials/thermal_expansion_strain1

The `eigenstrain_name` parameter value must also be set for the strain calculator, and an example
parameter setting is shown below:

!listing modules/tensor_mechanics/test/tests/ad_thermal_expansion_function/finite_const.i
         block=Modules/TensorMechanics/Master

!syntax parameters /Materials/ADComputeMeanThermalExpansionFunctionEigenstrain

!syntax inputs /Materials/ADComputeMeanThermalExpansionFunctionEigenstrain

!syntax children /Materials/ADComputeMeanThermalExpansionFunctionEigenstrain

!bibtex bibliography
