# StressBasedChemicalPotential

!syntax description /Materials/StressBasedChemicalPotential

The chemical potential $\mu$ is computed from the previous time step value of the stress tensor $\boldsymbol{\sigma}_{old}$,
the direction tensor $\boldsymbol{D}$ and a user-specified prefactor Real-value material property $f$.

!equation
\mu = -\boldsymbol{\sigma}_{old}::\boldsymbol{D} * f

The derivative of the chemical potential with regards to a concentration variable $c$ is only computed
if the [!param](/Materials/StressBasedChemicalPotential/c) parameter for the concentration variable is passed,
and only captures the dependency of $f$ on $c$.

!equation
\dfrac{\partial \mu}{\partial c} = -\boldsymbol{\sigma}_{old}::\boldsymbol{D} * \dfrac{\partial f}{\partial c}

!alert warning
Because the previous time step value of the stress is used, this coupling is naturally explicit and first order in time
with regards to the stress.

!syntax parameters /Materials/StressBasedChemicalPotential

!syntax inputs /Materials/StressBasedChemicalPotential

!syntax children /Materials/StressBasedChemicalPotential
