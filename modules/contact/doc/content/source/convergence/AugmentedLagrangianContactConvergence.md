# AugmentedLagrangianContactConvergence

The augmented Lagrangian contact algorithm involves a nested solution strategy; see [AugmentedLagrangianContactProblem.md]. 

The `AugmentedLagrangianContactConvergence` object tracks the convergence of the nested solution procedure described above, allows interaction with the solver at each iteration until the tolerance criteria are met, and allows for updating the Lagrangian multipliers as prescribed by [AugmentedLagrangianContactProblem.md]
