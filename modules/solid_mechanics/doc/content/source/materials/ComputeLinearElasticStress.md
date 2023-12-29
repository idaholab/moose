# Compute Linear Elastic Stress

!syntax description /Materials/ComputeLinearElasticStress

## Description

This material, `ComputeLinearElasticStress` computes the elastic stress for a total and small strain formulation: this stress class is compatible with the [ComputeSmallStrain](/ComputeSmallStrain.md) type of strain calculators, including those for non-Cartesian coordinate systems.
This stress calculator class can be used with any coordinate system to calculate the elastic stress response for a small total formulation strain.

Elastic materials do not experience permanent deformation, and all elastic strain and elastic stress is recoverable. Elastic stress is related to elastic strain through the elasticity tensor
\begin{equation}
\sigma_{ij} = C_{ijkl} \epsilon_{kl}^{total}
\end{equation}
where $\boldsymbol{\epsilon}^{total}$ is the total strain formulation; this strain measure is also the sum of the mechanical elastic strain and any eigenstrains in the system.

## Example Input File Syntax

!listing modules/tensor_mechanics/tutorials/basics/part_1.1.i block=Materials/stress


!syntax parameters /Materials/ComputeLinearElasticStress

!syntax inputs /Materials/ComputeLinearElasticStress

!syntax children /Materials/ComputeLinearElasticStress
