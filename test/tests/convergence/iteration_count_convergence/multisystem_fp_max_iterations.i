# Tests that IterationCountConvergence works for multi-system fixed-point iteration.
# Iteration indices start at 0, so 'max_iterations = 3' should have iteration indices
# {0, 1, 2} printed in the console output but not index 3.

!include ../../multisystem/picard/nonlinearfe_nonlinearfe/multi_system.i

[Convergence]
  [iteration_conv]
    type = IterationCountConvergence
    max_iterations = 3
    verbose = true
  []
[]

[Executioner]
  multi_system_fixed_point_convergence := iteration_conv
[]
