[StochasticTools]
[]

[FormFunction]
  type = QuadraticMinimize
  parameter_names = 'result1 result2 result3'
  num_values = '3 2 2'
  initial_condition = '5 4 2 9 5 1 0'

  objective = 1.0
  solution = '1 2 3 4 5 6 7'
[]

[Executioner]
  type = Optimize
  tao_solver = TAOCG
  solve_on = none
  verbose = true
[]

[Outputs]
  csv = true
[]
