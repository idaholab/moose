# ComputePlasticHeatEnergy

!syntax description /Materials/ComputePlasticHeatEnergy

The `ComputePlasticHeatEnergy` computes the energy dissipated through heat during plastic deformation.
The plastic heat energy $P$ is:

!equation
P = \boldsymbol{\sigma}::(\boldsymbol{\epsilon}_p - \boldsymbol{\epsilon}_{p,old}) / dt

where:

- $P$ is the plastic heat energy
- $\boldsymbol{\sigma}$ is the stress tensor
- $\boldsymbol{\epsilon}_p$ is the plastic strain tensor
- $\boldsymbol{\epsilon}_{p,old}$ is the plastic strain tensor at the previous time step
- $dt$ is the time step

The derivatives of the plastic heat energy with regards to the strain is computed by differentiating
the equation above.

!alert note
The derivatives of the plastic heat energy are only computed during the computation of the Jacobian,
as an optimization.

!alert warning
The computation of the plastic heat energy assumes a first order Euler time integration scheme.

!syntax parameters /Materials/ComputePlasticHeatEnergy

!syntax inputs /Materials/ComputePlasticHeatEnergy

!syntax children /Materials/ComputePlasticHeatEnergy
