# AddCoupledEqSpeciesActions

!syntax description /ReactionNetwork/AqueousEquilibriumReactions/AddCoupledEqSpeciesAction

This action only creates the [Kernels](Kernels/index.md) and [AuxKernels](AuxKernels/index.md).
The primary species are created as [nonlinear variables](Variables/index.md), by the [AddPrimarySpeciesAction.md].
The reactions are parsed from the syntax described on this [page](modules/chemical_reactions/index.md#parser).
Equilibrium species are output as [auxiliary variables](AuxVariables/index.md), by the [AddSecondarySpeciesAction.md].
This action is the aqueous equilibrium pendant of the solid kinetic [AddCoupledSolidKinSpeciesAction.md].

!syntax parameters /ReactionNetwork/AqueousEquilibriumReactions/AddCoupledEqSpeciesAction

!syntax inputs /ReactionNetwork/AqueousEquilibriumReactions/AddCoupledEqSpeciesAction

!syntax children /ReactionNetwork/AqueousEquilibriumReactions/AddCoupledEqSpeciesAction
