# PorousFlowWaterNCG

!syntax description /UserObjects/PorousFlowWaterNCG

The `compute_enthalpy` and `compute_internal_energy` parameters allow the enthalpy and internal
energy of each phase to be skipped. This is useful in isothermal simulations with no energy
equation, where these properties are not needed but would otherwise be computed at every
residual and Jacobian evaluation.

!syntax parameters /UserObjects/PorousFlowWaterNCG

!syntax inputs /UserObjects/PorousFlowWaterNCG

!syntax children /UserObjects/PorousFlowWaterNCG
