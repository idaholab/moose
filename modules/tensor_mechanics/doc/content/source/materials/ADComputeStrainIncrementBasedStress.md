# Compute Strain Increment Based Stress

!syntax description /Materials/ADComputeStrainIncrementBasedStress

## Description

This stress calculator finds the value of the stress as a function of the elastic
strain increment when a series of inelastic strains are specified in the input file.
The stress is calculated as
\begin{equation}
  \label{eqn:stress}
  \sigma_{ij} = \sigma_{ij}^{old} + C_{ijkl} \Delta \epsilon_{jk}^{el}
\end{equation}
where $\sigma_{ij}$ is the stress and $C_{ijkl}$ is the elasticity tensor of the
material.
The elastic strain increment, $\Delta \epsilon_{jk}^{el}$ is found by subtracting
the sum of the inelastic strains from the mechanical strain:
\begin{equation}
  \label{eqn:elastic_strain_incr}
  \Delta \boldsymbol{\epsilon}^{el} = \boldsymbol{\epsilon}^{mech} - \boldsymbol{\epsilon}^{mech-old}
      - \sum_n \left( \boldsymbol{\epsilon}^{inel}_n - {\boldsymbol{\epsilon}^{inel-old}}_n \right)
\end{equation}
where $\boldsymbol{\epsilon}^{mech}$ is the mechanical strain and
$\boldsymbol{\epsilon}^{inel}$ is the inelastic strain.
In the tensor mechanics module mechanical strain is defined as the sum of the
elastic and inelastic (e.g. creep and/or plasticity) strains.

## Example Input File

!listing modules/tensor_mechanics/test/tests/plane_stress/weak_plane_stress_incremental.i block=Materials/stress

!syntax parameters /Materials/ADComputeStrainIncrementBasedStress

!syntax inputs /Materials/ADComputeStrainIncrementBasedStress

!syntax children /Materials/ADComputeStrainIncrementBasedStress

!bibtex bibliography
