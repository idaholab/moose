[Optimization]
[]

[OptimizationReporter]
  type = QuadraticMinimize
  parameter_names = 'result1 result2 result3'
  num_values = '3 2 2'
  initial_condition = '5 4 2; 9 5; 1 0'
  objective = 1.0
  solution = '1 2 3 4 5 6 7'
  measurement_points = ''
  measurement_values = ''
[]

[Executioner]
  type = Optimize
  tao_solver = TAOBNCG
  petsc_options_iname = '-tao_gatol -tao_cg_delta_max'
  petsc_options_value = '1e-4 1e-2'
  solve_on = none
  verbose = true
[]

[Outputs]
  csv = true
[]
