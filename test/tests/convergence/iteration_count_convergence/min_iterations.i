!include base.i

[Convergence]
  [test_conv]
    convergence_statuses = '1 1 1 1 1 1 1'
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = min_iterations_out
    execute_on = 'FINAL'
  []
[]
