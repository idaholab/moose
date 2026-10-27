# PorousFlowFluidState

!syntax description /Materials/PorousFlowFluidState

Material that provides all thermophysical properties of the system given a set
of nonlinear variables using the formulation provided by one of the fluid state
UserObjects.

The primary variables are the gas porepressure, the temperature and the total mass fractions
$z_i$ of the fluid components summed over all phases. A fluid state with $N$ fluid components
(not counting salt) requires $N - 1$ total mass fractions in `z`: the $i$-th entry is the total
mass fraction of fluid component $i + 1$, and the total mass fraction of component 0 (the aqueous
component) is the remainder.

The salt mass fraction `xnacl` can only be supplied to fluid states that include salt, such as
[PorousFlowBrineCO2](PorousFlowBrineCO2.md). It may be a nonlinear variable or an AuxVariable.

!syntax parameters /Materials/PorousFlowFluidState

!syntax inputs /Materials/PorousFlowFluidState

!syntax children /Materials/PorousFlowFluidState
