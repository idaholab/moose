# PorousFlowBrineCO2

!syntax description /UserObjects/PorousFlowBrineCO2

A high precision equation of state for brine and CO$_2$, including the mutual solubility of
CO$_2$ into the liquid brine and water vapor into the CO$_2$-rich gas phase using the accurate
fugacity-based formulation of [!cite](spycher2003) and [!cite](spycher2005).

This model is suitable for simulations of geological storage of CO$_2$ in saline aquifers.

For more details, see the documentation of the [brine and CO$_2$](brineco2.md) equation of state.

The [!param](/UserObjects/PorousFlowBrineCO2/compute_enthalpy) and [!param](/UserObjects/PorousFlowBrineCO2/compute_internal_energy) parameters allow the enthalpy and internal
energy of each phase to be skipped. This is useful in isothermal simulations with no energy
equation, where these properties are not needed but would otherwise be computed at every
residual and Jacobian evaluation.

## Phase and component numbering

The phase and component numbering is fixed by this class and is no longer user-configurable. The
liquid phase is phase 0 and the gas phase is phase 1; water is component 0, CO$_2$ is component 1
and salt (NaCl) is component 2. The `liquid_phase_number`, `liquid_fluid_component` and
`salt_component` parameters are deprecated, and any value other than the fixed numbering above
is an error.

!syntax parameters /UserObjects/PorousFlowBrineCO2

!syntax inputs /UserObjects/PorousFlowBrineCO2

!syntax children /UserObjects/PorousFlowBrineCO2

!bibtex bibliography
