# Strain Energy Rate Density

!syntax description /Materials/StrainEnergyRateDensity

## Description

This material computes the strain energy rate density, $\dot{u}$, which is defined as the
time rate of the strain energy density (see [`StrainEnergyDensity`](/StrainEnergyDensity.md)):
\begin{equation}
  \label{eqn:sed_integral_def}
  \dot{u} = \int \boldsymbol{\sigma} : \textrm{d}\dot{\boldsymbol{\epsilon}}
\end{equation}
where $\boldsymbol{\sigma}$ is the stress tensor and $\dot{\boldsymbol{\epsilon}}$ is the strain rate. The material supplied through `inelastic_models` evaluates this expression. For example, the power-law creep model uses its analytical solution. Creep models derived from `RadialReturnCreepStressUpdateBase` can instead implement the equivalent creep strain rate and use the base class Gaussian quadrature in effective-stress space; `serd_integration_order` selects the quadrature order.

The strain energy rate density is primarily used to compute the C(t) integral, see [`FractureIntegrals`](/FractureIntegrals.md).

The strain rate here is the sum of the elastic and inelastic (e.g. plastic, creep) strain rates.

When `use_incremental_serd = true`, the material instead approximates the integral using the
trapezoidal rule along the effective stress--effective strain-rate path:
\begin{equation}
  \dot{u}_k = \dot{u}_{k-1} + \frac{1}{2}
  \left(\sigma_{\mathrm{eq},k} + \sigma_{\mathrm{eq},k-1}\right)
  \left(\dot{\epsilon}_{\mathrm{eq},k} - \dot{\epsilon}_{\mathrm{eq},k-1}\right).
\end{equation}
This option does not require `inelastic_models`. Its accuracy depends on resolving the
stress--strain-rate path with sufficiently small time steps.

This class is available both for manually coded Jacobian and automatic differentiation strategies.

## Example Input File

!listing modules/solid_mechanics/test/tests/strain_energy_density/ad_rate_model_weak_plane.i block=Materials/strain_energy_rate_density

!syntax parameters /Materials/StrainEnergyRateDensity

!syntax inputs /Materials/StrainEnergyRateDensity

!syntax children /Materials/StrainEnergyRateDensity

!bibtex bibliography
