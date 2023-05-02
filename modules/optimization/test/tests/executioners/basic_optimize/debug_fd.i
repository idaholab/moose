[Optimization]
[]

[OptimizationReporter]
  type = QuadraticMinimize
  parameter_names = 'results'
  num_values = 3
  initial_condition = '5 8 1'
  objective = 0.0
  solution = '1 2 3'
  measurement_points = ''
  measurement_values = ''
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname='-tao_fd_gradient -tao_fd_delta -tao_gatol'
  petsc_options_value='true 1e-8 0.1'
  verbose = true
[]
