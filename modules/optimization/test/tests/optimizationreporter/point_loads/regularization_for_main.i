[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'parameter_results'
  num_values = '3'
  tikhonov_coeff = 0.0001
  measurement_points = '0.5 0.28 0
                        0.5 1.1 0'
  measurement_values = '293 320'
[]

[Outputs]
  file_base = main_out_reg
  csv = true
[]
