# Strain Energy Density

!syntax description /Materials/StrainEnergyDensity

## Description

This material computes the strain energy density, $u$, which is defined as the
area underneath the stress-strain curve:
\begin{equation}
  \label{eqn:sed_integral_def}
  u = \int_x \frac{1}{2} \sigma : d \epsilon
\end{equation}
where $\sigma$ is the stress and $\epsilon$ is the mechanical strain. In the
tensor mechanics module we define the mechanical strain as the sum of the
elastic and inelastic (e.g. plastic, creep) strain without the eigenstrains.

The strain energy density can be calculated either in total or in incremental
form, based on the strain measured applied.
In the incremental form the strain energy density integral takes the form
\begin{equation}
  \label{eqn:incremental_sed}
  u = u_{old} + \frac{1}{2} \sigma : \Delta \epsilon +
      \frac{1}{2}\sigma_{old} : \Delta \epsilon
\end{equation}
where $\Delta \epsilon$ is the mechancial strain increment.

!alert note title=Monotonic Loading Only
The +`StrainEnergyDensity`+ class is formulated only for monotonic loading and
should not be used to calculate the strain energy density for cyclic loading cases.

## Example Input File

!listing modules/tensor_mechanics/test/tests/strain_energy_density/incr_model_elas_plas.i block=Materials/strain_energy_density

!syntax parameters /Materials/StrainEnergyDensity

!syntax inputs /Materials/StrainEnergyDensity

!syntax children /Materials/StrainEnergyDensity

!bibtex bibliography
