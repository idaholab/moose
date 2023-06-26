[Optimization]
[]

[OptimizationReporter]
  type = QuadraticMinimizeConstrained
  parameter_names = 'results'
  num_values = 3
  initial_condition = '5 8 1'
  objective = 0.0
  solution = '1 2 3'
  measurement_points = ''
  measurement_values = ''
  equality_names = 'equal'

[]

[Executioner]
  type = Optimize
  tao_solver = taoalmm
  solve_on = none
  verbose = true
  petsc_options_iname = '-tao_gttol -tao_catol'
  petsc_options_value = ' 3e-5 1e-2'
[]

[Outputs]
  csv = true
[]
