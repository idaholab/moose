[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  num_values = 2
  initial_condition = '35 35'
  tikhonov_coeff = 10
  lower_bounds = '1'
  upper_bounds = '50'
[]

[Outputs]
  file_base = main_out_reg
  csv = true
[]
