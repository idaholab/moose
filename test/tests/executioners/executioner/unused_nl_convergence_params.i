!include steady.i

[Convergence]
  [test_conv]
    type = SuppliedStatusConvergence
    min_iterations = 2
    max_iterations = 4
    convergence_statuses = '0 0 0 0 0 0 0'
    converge_at_max_iterations = true
  []
[]

[Executioner]
  nonlinear_convergence = test_conv
  nl_max_its = 3
[]
