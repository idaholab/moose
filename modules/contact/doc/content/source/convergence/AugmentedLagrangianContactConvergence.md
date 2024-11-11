# AugmentedLagrangianContactConvergence

The augmented Lagrangian contact algorithm involves a nested solution strategy; see [AugmentedLagrangianContactProblem.md].

`AugmentedLagrangianContactFEProblemConvergence` and  `AugmentedLagrangianContactReferenceConvergence`
track the convergence of the nested solution procedure described above, allow interaction with the solver at each iteration until the tolerance criteria are met, and allow for updating the Lagrangian multipliers as prescribed by [AugmentedLagrangianContactProblem.md].

## AugmentedLagrangianContactFEProblemConvergence

!syntax description /Convergence/AugmentedLagrangianContactFEProblemConvergence

!syntax parameters /Convergence/AugmentedLagrangianContactFEProblemConvergence

!syntax inputs /Convergence/AugmentedLagrangianContactFEProblemConvergence

!syntax children /Convergence/AugmentedLagrangianContactFEProblemConvergence

## AugmentedLagrangianContactReferenceConvergence

!syntax description /Convergence/AugmentedLagrangianContactReferenceConvergence

!syntax parameters /Convergence/AugmentedLagrangianContactReferenceConvergence

!syntax inputs /Convergence/AugmentedLagrangianContactReferenceConvergence

!syntax children /Convergence/AugmentedLagrangianContactReferenceConvergence
