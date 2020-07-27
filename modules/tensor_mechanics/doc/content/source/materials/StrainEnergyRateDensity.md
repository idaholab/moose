# Strain Energy Rate Density

!syntax description /Materials/StrainEnergyRateDensity

## Description

This material computes the strain energy rate density, $\dot{u}$, which is defined as the
time rate of the strain energy density (see [`StrainEnergyDensity`](/StrainEnergyDensity.md)):
\begin{equation}
  \label{eqn:sed_integral_def}
  \dot{u} = \int \boldsymbol{\sigma} : \textrm{d}\dot{\boldsymbol{\epsilon}}
\end{equation}
where $\boldsymbol{\sigma}$ is the stress tensor and $\dot{\boldsymbol{\epsilon}}$ is the strain rate. This expression is multiplied by $\frac{n}{n+1}$, where $n$ is the power law exponent of the material provided though the `inelastic_models` input parameter. This factor decreases the strain energy rate density to better capture the strain rate field around a crack under steady-state creep growth. This factor is primarily used to compute the C(t) integral, see [`FractureIntegrals`](/FractureIntegrals.md).

The strain rate here is the sum of the elastic and inelastic (e.g. plastic, creep) strain rates.

This class is available both for manually coded Jacobian and automatic differentiation strategies.

## Example Input File

!listing modules/tensor_mechanics/test/tests/strain_energy_density/ad_rate_model_weak_plane.i block=Materials/strain_energy_rate_density

!syntax parameters /Materials/StrainEnergyRateDensity

!syntax inputs /Materials/StrainEnergyRateDensity

!syntax children /Materials/StrainEnergyRateDensity

!bibtex bibliography
