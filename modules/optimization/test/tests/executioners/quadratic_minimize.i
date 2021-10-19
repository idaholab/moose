[StochasticTools]
[]

[FormFunction]
  type = QuadraticMinimize
  parameter_names = 'results'
  num_values = 3
  initial_condition = '5 8 1'
  misfit_name = 'not_used_for_this_problem'
  objective = 1.0
  solution = '1 2 3'
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
