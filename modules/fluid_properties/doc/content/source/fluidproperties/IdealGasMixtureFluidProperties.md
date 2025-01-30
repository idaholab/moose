# IdealGasMixtureFluidProperties

This class computes fluid properties for a mixture of ideal gases (see [IdealGasFluidProperties.md]).

First note the following definitions and relations:

!equation
\xi_k = \frac{m_k}{m} \,,

!equation
\psi_k = \frac{n_k}{n} \,,

!equation
M = \frac{1}{\sum\limits_k \frac{\xi_k}{M_k}} = \sum\limits_k \psi_k M_k \,,

!equation
\xi_k = \frac{M_k}{M} \psi_k \,,

where

- $m_k$ is the mass of component $k$,
- $m = \sum_k m_k$ is the mixture mass,
- $n_k$ is the number of moles of component $k$,
- $n = \sum_k n_k$ is the mixture number of moles,
- $M_k$ is the molar mass of component $k$, and
- $M$ is the mixture molar mass.

Due to Dalton's law of partial pressures,

!equation
p_k = \psi_k p \,,

where $p_k$ is the partial pressure of component $k$ and $p$ is the mixture pressure.

Mixture mass-specific properties, such as specific volume $v$, specific internal
energy $e$, and specific entropy $s$ are computed as mass-fraction-weighted
sums of the component properties evaluated at the mixture temperature and pressure:

!equation
z(p, T) = \sum\limits_k \xi_k z_k(p, T) \,.

Transport properties such as dynamic viscosity $\mu$ and thermal conductivity $k$ are
computed as molar-fraction-weighted
sums of the component properties evaluated at the mixture temperature and pressure:

!equation
y(p, T) = \sum\limits_k \psi_k y_k(p, T) \,.

!syntax parameters /FluidProperties/IdealGasMixtureFluidProperties

!syntax inputs /FluidProperties/IdealGasMixtureFluidProperties

!syntax children /FluidProperties/IdealGasMixtureFluidProperties
