[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = diffusivity_values
  num_values = 2
  initial_condition = '35 35'
  tikhonov_coeff = 10
  lower_bounds = '1'
  upper_bounds = '50'
  measurement_file = 'synthetic_data.csv'
  file_value = 'temperature'
[]

[Outputs]
  file_base = main_out_reg
  csv = true
[]
