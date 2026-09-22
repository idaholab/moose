# PorousFlowBrineCO2

!syntax description /UserObjects/PorousFlowBrineCO2

A high precision equation of state for brine and CO$_2$, including the mutual solubility of
CO$_2$ into the liquid brine and water vapor into the CO$_2$-rich gas phase using the accurate
fugacity-based formulation of [!cite](spycher2003) and [!cite](spycher2005).

This model is suitable for simulations of geological storage of CO$_2$ in saline aquifers.

For more details, see the documentation of the [brine and CO$_2$](brineco2.md) equation of state.

The `compute_enthalpy` and `compute_internal_energy` parameters allow the enthalpy and internal
energy of each phase to be skipped. This is useful in isothermal simulations with no energy
equation, where these properties are not needed but would otherwise be computed at every
residual and Jacobian evaluation.

!syntax parameters /UserObjects/PorousFlowBrineCO2

!syntax inputs /UserObjects/PorousFlowBrineCO2

!syntax children /UserObjects/PorousFlowBrineCO2

!bibtex bibliography
