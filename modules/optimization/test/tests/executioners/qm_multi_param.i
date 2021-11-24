[StochasticTools]
[]

[OptimizationReporter]
  type = QuadraticMinimize
  parameter_names = 'result1 result2 result3'
  num_values = '3 2 2'
  initial_condition = '5 4 2 9 5 1 0'
  objective = 1.0
  solution = '1 2 3 4 5 6 7'
  measurement_points = ''
  measurement_values = ''
[]

[Executioner]
  type = Optimize
  tao_solver = TAONTR
  petsc_options_iname='-tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type'
  petsc_options_value='0 1e16 constant'
  solve_on = none
  verbose = true
[]

[Outputs]
  csv = true
[]
