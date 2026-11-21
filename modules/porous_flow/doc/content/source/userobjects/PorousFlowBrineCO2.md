# PorousFlowBrineCO2

!syntax description /UserObjects/PorousFlowBrineCO2

A high precision equation of state for brine and CO$_2$, including the mutual solubility of
CO$_2$ into the liquid brine and water vapor into the CO$_2$-rich gas phase using the accurate
fugacity-based formulation of [!cite](spycher2003) and [!cite](spycher2005).

This model is suitable for simulations of geological storage of CO$_2$ in saline aquifers.

For more details, see the documentation of the [brine and CO$_2$](brineco2.md) equation of state.

In a single-phase state the absent phase has zero saturation, and therefore no mass and no
advective flux. Its density, viscosity and composition are nevertheless computed, from the
equilibrium state at the current pressure and temperature: in a liquid-only state the gas phase is
given the density and viscosity of CO$_2$ and the equilibrium gas composition, and in a gas-only
state the liquid phase is given the properties of CO$_2$-saturated brine. These properties
therefore vary continuously when a phase appears or disappears. This matters for terms that are
not weighted by the saturation, such as the gravity head of a face in the finite-volume kernels
and the phase composition in the diffusive flux, which would otherwise jump at the phase boundary
and prevent Newton from converging there.

The [!param](/UserObjects/PorousFlowBrineCO2/compute_enthalpy) and [!param](/UserObjects/PorousFlowBrineCO2/compute_internal_energy) parameters allow the enthalpy and internal
energy of each phase to be skipped. This is useful in isothermal simulations with no energy
equation, where these properties are not needed but would otherwise be computed at every
residual and Jacobian evaluation.

!syntax parameters /UserObjects/PorousFlowBrineCO2

!syntax inputs /UserObjects/PorousFlowBrineCO2

!syntax children /UserObjects/PorousFlowBrineCO2

!bibtex bibliography
