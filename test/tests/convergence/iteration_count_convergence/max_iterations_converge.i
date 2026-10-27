!include base.i

[Convergence]
  [test_conv]
    convergence_statuses = '0 0 0 0 0 0 0'
    converge_at_max_iterations = true
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = max_iterations_converge_out
    execute_on = 'FINAL'
  []
[]
