# Strain Energy Rate Density

!syntax description /Materials/StrainEnergyRateDensity

## Description

This material computes the strain energy rate density, $\dot{u}$, which is defined as the
time rate of the strain energy density (see [`StrainEnergyDensity`](/StrainEnergyDensity.md)):
\begin{equation}
  \label{eqn:sed_integral_def}
  \dot{u} = \sigma : \dot{\epsilon}
\end{equation}
where $\sigma$ is the stress and $\epsilon$ is the strain rate. In the
tensor mechanics module we define the strain rate as the sum of the
elastic and inelastic (e.g. plastic, creep) strain rates.

!alert note title=Time discretization error
The +`StrainEnergyRateDensity`+ class uses time increments to obtain strain rates. For this reason,
transient simulations must take small steps to guarantee converged results. Inaccurate results may
lead to misleading postprocessing values, such as the analysis of crack creep deformation via
the C(t) integral.

## Example Input File

!listing modules/tensor_mechanics/test/tests/strain_energy_density/rate_model_elas_plas.i block=Materials/strain_energy_rate_density

!syntax parameters /Materials/StrainEnergyRateDensity

!syntax inputs /Materials/StrainEnergyRateDensity

!syntax children /Materials/StrainEnergyRateDensity

!bibtex bibliography
