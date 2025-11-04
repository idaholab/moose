!include transient.i

[Convergence]
  [iteration_conv]
    type = IterationCountConvergence
    max_iterations = 3
  []
[]

[Executioner]
  steady_state_detection = true
  steady_state_convergence = iteration_conv
[]
