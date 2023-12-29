# ADComputeFiniteStrainElasticStress

!syntax description /Materials/ADComputeFiniteStrainElasticStress

## Description

This material, `ADComputeFiniteStrainElasticStress` computes the elastic stress
for an incremental formulation, both incremental small
([ADComputeIncrementalSmallStrain](/ADComputeIncrementalSmallStrain.md) type) and
incremental finite ([ADComputeFiniteStrain](/ADComputeFiniteStrain.md) type)
strain formulations. This stress class is compatible with both Cartesian and
non-Cartesian strain calculators, including
[Axisymmetric](/ADComputeAxisymmetricRZFiniteStrain.md) and
[RSpherical](/ADComputeRSphericalFiniteStrain.md).

Elastic materials do not experience permanent deformation, and all elastic
strain and elastic stress is recoverable. Elastic stress is related to elastic
strain through the elasticity tensor

\begin{equation}
  \label{eq:incremental_stress_calculator}
  \sigma_{ij} = C_{ijkl} \Delta \epsilon_{kl}
\end{equation}

where $\Delta \boldsymbol{\epsilon}$ is the strain increment; this strain
measure is also the sum of the mechanical elastic strain and any eigenstrains in
the system.

!syntax parameters /Materials/ADComputeFiniteStrainElasticStress

!syntax inputs /Materials/ADComputeFiniteStrainElasticStress

!syntax children /Materials/ADComputeFiniteStrainElasticStress
