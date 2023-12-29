# Compute Extra Stress Constant

!syntax description /Materials/ComputeExtraStressConstant

## Description

The class `ComputeExtraStressConstant` adds an additional stress term, ($\boldsymbol{\sigma}_0$), to
the residual calculation after the constitutive model calculation of the stress, as shown in
[eq:extra_stress_addition].  An extra stress may be a residual stress, such as in large civil
engineering simulations.

The extra stress material property, `extra_stress` stores the Rank-2 tensor values of the extra stress.
\begin{equation}
  \label{eq:extra_stress_addition}
  \sigma_{ij} = \sigma_{ij} + \sigma^{extra}_{ij}
\end{equation}
where the value of $\boldsymbol{\sigma}^{extra}$ is constant across the entire mesh in this class.

`ComputeExtraStressConstant` creates a symmetric stress tensor, and expects the values of the stress
tensor components in the specific order specified in the input parameter description below.

## Example Input File Syntax

!listing modules/combined/test/tests/linear_elasticity/extra_stress.i block=Materials/const_stress

!syntax parameters /Materials/ComputeExtraStressConstant

!syntax inputs /Materials/ComputeExtraStressConstant

!syntax children /Materials/ComputeExtraStressConstant
