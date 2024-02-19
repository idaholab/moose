[Optimization]
[]

[OptimizationReporter]
  type = QuadraticMinimizeConstrained
  parameter_names = 'results'
  num_values = 3
  initial_condition = '1 1 1'
  objective = 0.0
  solution = '1 2 3'
  solution_sum_equality = 5.0
  measurement_points = ''
  measurement_values = ''
  equality_names = 'equal'

[]

[Executioner]
  type = Optimize
  tao_solver = taoalmm
  solve_on = none
  verbose = true
  petsc_options_iname = '-tao_gttol -tao_catol  -tao_almm_type -tao_almm_mu_factor '
  petsc_options_value = ' 3e-5 1e-4  phr  1.1'

[]

[Outputs]
  csv = true
[]
