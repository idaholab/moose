# IdealGasMixtureFluidProperties

This is a [VaporMixtureFluidProperties.md] where each of the components are assumed to be ideal gases (see [IdealGasFluidProperties.md]).

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

Mixture mass-specific properties, such as specific volume $v$ and specific internal
energy $e$ are computed as mass-fraction-weighted
sums of the component properties evaluated at the mixture temperature and pressure:

!equation
z(p, T) = \sum\limits_k \xi_k z_k(p, T) \,.

Mixture entropy is an exception to this rule, as the component entropies must be
evaluated at their partial pressures, rather than the mixture pressure, in accordance
with the Gibbs' theorem, which accounts for the entropy of mixing [!citep](cengel2008thermodynamics):

!equation
s(p, T) = \sum\limits_k \xi_k s_k(p_k, T) \,.

The temperature is computed from the specific volume $v$ and the specific internal energy $e$
by solving

!equation
e = \sum\limits_k \xi_k e_k(T)

for $T$ with the 1D Newton solver of the [fluid properties utilities](utils/FluidPropertiesUtils.md);
the internal energy of an ideal gas does not depend on pressure. The equation is offset by a
constant that makes both of its sides close to $c_v T$, where $c_v$ is the mixture isochoric
specific heat, so that the convergence test of the solver bounds the relative error in temperature
even where the internal energy is close to zero. The pressure then follows from the ideal gas law,
$p = R T / (M v)$, where $R$ is the universal gas constant.

Transport properties such as dynamic viscosity $\mu$ and thermal conductivity $k$ are
computed as molar-fraction-weighted
sums of the component properties evaluated at the mixture temperature and pressure:

!equation
y(p, T) = \sum\limits_k \psi_k y_k(p, T) \,.

!syntax parameters /FluidProperties/IdealGasMixtureFluidProperties

!syntax inputs /FluidProperties/IdealGasMixtureFluidProperties

!syntax children /FluidProperties/IdealGasMixtureFluidProperties
