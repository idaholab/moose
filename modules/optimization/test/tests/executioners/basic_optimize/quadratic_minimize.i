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
  solve_on = none
  verbose = true
[]

[Outputs]
  csv = true
[]
