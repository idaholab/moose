# PorousFlowWaterNCG

!syntax description /UserObjects/PorousFlowWaterNCG

The [!param](/UserObjects/PorousFlowWaterNCG/compute_enthalpy) and [!param](/UserObjects/PorousFlowWaterNCG/compute_internal_energy) parameters allow the enthalpy and internal energy of each phase to be skipped. This is useful in isothermal simulations with no energy
equation, where these properties are not needed but would otherwise be computed at every residual and Jacobian evaluation.

## Phase and component numbering

The phase and component numbering is fixed by this class and is no longer user-configurable. The
liquid phase is phase 0 and the gas phase is phase 1; water is component 0 and the non-condensable
gas is component 1. There is no salt component. The `liquid_phase_number` and
`liquid_fluid_component` parameters are deprecated, and any value other than 0 is an error.

!syntax parameters /UserObjects/PorousFlowWaterNCG

!syntax inputs /UserObjects/PorousFlowWaterNCG

!syntax children /UserObjects/PorousFlowWaterNCG
