# GeochemistryQuantityAux

This AuxKernel is used to extract information from the GeochemistryReactor that underlies `geochemistry` models.

A large number of AuxVariables and GeochemistryQuantityAux Kernels are often added automatically using the `geochemistry` actions, for instance [TimeDependentReactionSolver](TimeDependentReactionSolver/index.md).  However, if users wish to add their own manually, an example (that adds an AuxVariable to measure pH) is

!listing modules/geochemistry/test/tests/geochemistry_quantity_aux/neglog10a.i start=[AuxVariables] end=[Postprocessors]

!syntax parameters /AuxKernels/GeochemistryQuantityAux

!syntax inputs /AuxKernels/GeochemistryQuantityAux

!syntax children /AuxKernels/GeochemistryQuantityAux

