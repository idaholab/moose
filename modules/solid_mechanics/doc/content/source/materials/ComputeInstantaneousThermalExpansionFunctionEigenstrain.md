# ComputeInstantaneousThermalExpansionFunctionEigenstrain

!syntax description /Materials/ComputeInstantaneousThermalExpansionFunctionEigenstrain

## Description

This model computes the eigenstrain tensor resulting from isotropic thermal expansion where the
temperature-dependent thermal expansion is defined by a user-supplied function that describes the
instantaneous thermal expansion coefficient $\alpha$ as a function of temperature, $T$.  Using a
trapezoidal rule to perform time integration of this function, the current value of the thermal
eigenstrain tensor, $\boldsymbol{\epsilon}^{th}_{t+1}$ is computed at a given time as:
\begin{equation}
\boldsymbol{\epsilon}^{th}_{t+1} = \boldsymbol{\epsilon}^{th}_{t} + \frac{1}{2}(\alpha_{(T_t)} + \alpha_{(T_{t+1})}) (T_{t+1}-T_{t}) \boldsymbol{I}
\end{equation}
where $t+1$ denotes quantities at the new step, and $t$ denotes quantities at the previous step, $T$
is the temperature, $\alpha_{(T)}$ is the instantaneous thermal expansion at a given temperature, and
$\boldsymbol{I}$ is the identity matrix. On the first step, the stress-free temperature is used as the
previous step's temperature.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/thermal_expansion_function/finite_const.i
         block=Materials/thermal_expansion_strain1

The `eigenstrain_name` parameter value must also be set for the strain calculator, and an example
parameter setting is shown below:

!listing modules/tensor_mechanics/test/tests/thermal_expansion_function/finite_const.i
         block=Modules/TensorMechanics/Master

!syntax parameters /Materials/ComputeInstantaneousThermalExpansionFunctionEigenstrain

!syntax inputs /Materials/ComputeInstantaneousThermalExpansionFunctionEigenstrain

!syntax children /Materials/ComputeInstantaneousThermalExpansionFunctionEigenstrain
