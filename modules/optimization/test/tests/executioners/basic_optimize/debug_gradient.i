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
  tao_solver = taobncg
  petsc_options_iname='-tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta'
  petsc_options_value='true true false 1e-8'
  petsc_options = '-tao_test_gradient_view'
  verbose = true
[]
