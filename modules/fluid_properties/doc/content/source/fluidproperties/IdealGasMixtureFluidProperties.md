# IdealGasMixtureFluidProperties

This is a [VaporMixtureFluidProperties.md] where each of the components are assumed to be ideal gases,
such as [IdealGasFluidProperties.md] or [NASA7IdealGasFluidProperties.md]. A fluid properties class
can be a component if it implements `IdealGasMixtureInterface`, which tells the mixture whether its
specific heats are constant and at which temperature Newton's method should start.

!alert note
The internal energies of these two kinds of components have different references: that of
[IdealGasFluidProperties.md] is $c_v T$ plus a user-supplied reference value, whereas that of
[NASA7IdealGasFluidProperties.md] includes the enthalpy of formation. Only the latter accounts
for the heat of reaction when the composition changes by reaction.

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

for $T$; the internal energy of an ideal gas does not depend on pressure. When all components have
constant specific heats, the internal energy is linear in temperature and $T$ follows explicitly.
Otherwise, $T$ is found with the 1D Newton solver of the
[fluid properties utilities](utils/FluidPropertiesUtils.md), starting at the lowest of the start
temperatures of the components whose specific heats are not constant. For NASA 7-coefficient
components, these are the upper ends of the temperature ranges of their data, above the solution,
for the reasons given in [NASA7IdealGasFluidProperties.md]. The equation is offset by a constant that makes both of its
sides close to $c_v T$, where $c_v$ is the mixture isochoric specific heat, so that the convergence
test of the solver bounds the relative error in temperature even where the internal energy is close
to zero. The pressure then follows from the ideal gas law, $p = R T / (M v)$, where $R$ is the
universal gas constant.

Transport properties such as dynamic viscosity $\mu$ and thermal conductivity $k$ are
computed as molar-fraction-weighted
sums of the component properties evaluated at the mixture temperature and pressure:

!equation
y(p, T) = \sum\limits_k \psi_k y_k(p, T) \,.

[NASA7IdealGasFluidProperties.md] does not yet provide viscosity or thermal conductivity, so these
properties are not yet available for mixtures with its components, and neither are flow models that
need them, such as the gas-mixture flow channels of the thermal hydraulics module.

!syntax parameters /FluidProperties/IdealGasMixtureFluidProperties

!syntax inputs /FluidProperties/IdealGasMixtureFluidProperties

!syntax children /FluidProperties/IdealGasMixtureFluidProperties
