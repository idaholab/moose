# AugmentedLagrangianContactProblem

The augmented Lagrangian contact algorithm involves a nested solution strategy. In the inner solve, the Lagrangian multipliers are kept fixed and the contact problem is solved. Once the problem is solved, the algorithm checks to see whether the constraints are satisfied and decides if convergence has been reached. If the model has not yet converged, the Lagrangian multipliers are updated, and the inner solve is repeated.

The AugmentedLagrangianContactProblem manages the nested solution procedure described above. Each inner solve is an ordinary nonlinear solve with its own nonlinear convergence check, iteration limits, and line search. After a converged inner solve, the constraints are checked; if they are not met, the multipliers are updated and the next inner solve starts from the current solution. The time step fails if an inner solve fails or if `maximum_lagrangian_update_iterations` multiplier updates do not meet the constraints. The nonlinear and linear iteration counts reported for the time step are the totals over all inner solves.

# AugmentedLagrangianContactProblem

!syntax description /Problem/AugmentedLagrangianContactProblem

!syntax parameters /Problem/AugmentedLagrangianContactProblem

!syntax inputs /Problem/AugmentedLagrangianContactProblem

!syntax children /Problem/AugmentedLagrangianContactProblem
