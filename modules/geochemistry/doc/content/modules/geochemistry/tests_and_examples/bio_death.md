# Simulating microbe mortality

It is important to capture microbe mortality in some simulations.  This usually follows an exponential decay
\begin{equation}
\label{eqn.mortality}
\frac{\mathrm{d}}{\mathrm{d}t}\mathrm{mass} = -k\times \mathrm{mass} \ ,
\end{equation}
where $k$ is the death rate.  In the `geochemistry` module, kinetic species such as microbes are parameterised by their mole number, so the above equation reads:
\begin{equation}
\frac{\mathrm{d}}{\mathrm{d}t}\mathrm{moles} = -k\times \mathrm{moles} = -\frac{k}{\mathrm{molar\ mass}}{\mathrm{mass}} \ .
\end{equation}

As discussed on the main [biogeochemistry](theory/biogeochemistry.md) page, this can only be implemented in the `geochemistry` module by making the microbe a kinetic species.  Then a [GeochemistryKineticRate](GeochemistryKineticRate.md) `UserObject` may be used to simulate mortality by setting:

- `intrinsic_rate_constant = k/molar_mass`
- `multiply_by_mass = true`
- `eta = 0`
- `direction = DEATH`.

!listing modules/geochemistry/test/tests/kinetics/bio_death.i block=rate_biomass_death

!media bio_death.png caption=Mortality of microbe using [eqn.mortality]  id=bio_death.fig








