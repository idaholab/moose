# TwoPhaseNCGPartialPressureFluidProperties

This fluid properties class is used for two-phase fluids that have a single,
non-condensable gas (NCG) mixed with their vapor phase. The vapor mixture model
of [IdealRealGasMixtureFluidProperties.md] is used, which corresponds to a
partial pressure mixture model for real gases.

In addition to the interfaces of its parent class, this class provides an
interface for computing the mass fraction of the NCG $\xi_\text{NCG}$ assuming saturation
of the main fluid at the temperature $T$ and mixture pressure $p$.
The molar fraction of the NCG $\psi_\text{NCG}$ is computed by combining the two relations:

!equation
p = p_{v,\text{sat}}(T) + p_\text{NCG} \,,

!equation
p_\text{NCG} = \psi_\text{NCG} p \,,

which with the relation between molar fractions and mass fractions, yields

!equation
\psi_\text{NCG} = \frac{p - p_{v,\text{sat}}(T)}{p} \,,

!equation
\xi_\text{NCG} = \frac{M_\text{NCG}}{M} \,,

where $M_\text{NCG}$ is the NCG molar mass, and $M$ is the mixture molar mass,

!equation
M = (1 - \psi_\text{NCG}) M_v + \psi_\text{NCG} M_\text{NCG} \,,

with $M_v$ denoting the main fluid molar mass.

!syntax parameters /FluidProperties/TwoPhaseNCGPartialPressureFluidProperties

!syntax inputs /FluidProperties/TwoPhaseNCGPartialPressureFluidProperties

!syntax children /FluidProperties/TwoPhaseNCGPartialPressureFluidProperties

!bibtex bibliography
