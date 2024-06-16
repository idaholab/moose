# Compute Finite Strain Elastic Stress

!syntax description /Materials/ComputeFiniteStrainElasticStress

## Description

This material, `ComputeFiniteStrainElasticStress` computes the elastic stress for an incremental formulation, both incremental small ([ComputeIncrementalStrain](/ComputeIncrementalStrain.md) type) and incremental finite ([ComputeFiniteStrain](/ComputeFiniteStrain.md) type) strain formulations.
This stress class is compatible with both Cartesian and non-Cartesian strain calculators, including [Axisymmetric](/ComputeAxisymmetricRZFiniteStrain.md) and [RSpherical](/ComputeRSphericalFiniteStrain.md).

Elastic materials do not experience permanent deformation, and all elastic strain and elastic stress is recoverable. Elastic stress is related to elastic strain through the elasticity tensor
\begin{equation}
  \label{eq:incremental_stress_calculator}
  \sigma_{ij} = C_{ijkl} \Delta \epsilon_{kl}
\end{equation}
where $\Delta \boldsymbol{\epsilon}$ is the strain increment; this strain measure is also the sum of the mechanical elastic strain and any eigenstrains in the system.


## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Materials/stress

!syntax parameters /Materials/ComputeFiniteStrainElasticStress

!syntax inputs /Materials/ComputeFiniteStrainElasticStress

!syntax children /Materials/ComputeFiniteStrainElasticStress
