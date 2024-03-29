# AddCoupledSolidKinSpeciesActions

!syntax description /ReactionNetwork/SolidKineticReactions/AddCoupledSolidKinSpeciesAction

This action only creates the [Kernels](Kernels/index.md) and [AuxKernels](AuxKernels/index.md).
The primary species are created as [nonlinear variables](Variables/index.md), by the [AddPrimarySpeciesAction.md].
The reactions are parsed from the syntax described on this [page](modules/chemical_reactions/index.md#parser).
Equilibrium species are output as [auxiliary variables](AuxVariables/index.md), by the [AddSecondarySpeciesAction.md].
This action is the solid kinetic pendant of the aqueous equilibrium [AddCoupledEqSpeciesAction.md].

!syntax parameters /ReactionNetwork/SolidKineticReactions/AddCoupledSolidKinSpeciesAction

!syntax inputs /ReactionNetwork/SolidKineticReactions/AddCoupledSolidKinSpeciesAction

!syntax children /ReactionNetwork/SolidKineticReactions/AddCoupledSolidKinSpeciesAction
