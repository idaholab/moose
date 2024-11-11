# AugmentedLagrangianContactProblem

The augmented Lagrangian contact algorithm involves a nested solution strategy. In the inner solve, the Lagrangian multipliers are kept fixed and the contact problem is solved. Once the problem is solved, the algorithm checks to see whether the constraints are satisfied and decides if convergence has been reached. If the model has not yet converged, the Lagrangian multipliers are updated, and the inner solve is repeated.

The AugmentedLagrangianContactProblem manages the nested solution procedure described above, repeating the solution until convergence has been achieved, which is controlled by [AugmentedLagrangianContactConvergence.md], and updating the Lagrangian multipliers.

# AugmentedLagrangianContactProblem

!syntax description /Problem/AugmentedLagrangianContactProblem

!syntax parameters /Problem/AugmentedLagrangianContactProblem

!syntax inputs /Problem/AugmentedLagrangianContactProblem

!syntax children /Problem/AugmentedLagrangianContactProblem
