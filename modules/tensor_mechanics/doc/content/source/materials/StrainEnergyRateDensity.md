# Strain Energy Rate Density

!syntax description /Materials/StrainEnergyRateDensity

## Description

This material computes the strain energy rate density, $\dot{u}$, which is defined as the
time rate of the strain energy density (see [`StrainEnergyDensity`](/StrainEnergyDensity.md)):
\begin{equation}
  \label{eqn:sed_integral_def}
  \dot{u} = \int \boldsymbol{\sigma} : \textrm{d}\dot{\boldsymbol{\epsilon}}
\end{equation}
where $\boldsymbol{\sigma}$ is the stress tensor and $\dot{\boldsymbol{\epsilon}}$ is the strain rate. This expression is multiplied by $\frac{n}{n+1}$ when the input argument `n_exponent` is supplied. This factor decreases the strain energy rate density to better capture the strain rate field around a crack under steady-state creep growth. 

In the tensor mechanics module we define the strain rate as the sum of the
elastic and inelastic (e.g. plastic, creep) strain rates.

!alert note title=Time discretization error
The +`StrainEnergyRateDensity`+ class uses time increments to obtain strain rates. For this reason, transient simulations must take small steps to guarantee converged results. Inaccurate results may lead to misleading postprocessing values, such as the analysis of crack creep deformation via
the C(t) integral, see [`FractureIntegrals`](/FractureIntegrals.md).

## Example Input File

!listing modules/tensor_mechanics/test/tests/strain_energy_density/rate_model_elas_plas.i block=Materials/strain_energy_rate_density

!syntax parameters /Materials/StrainEnergyRateDensity

!syntax inputs /Materials/StrainEnergyRateDensity

!syntax children /Materials/StrainEnergyRateDensity

!bibtex bibliography
