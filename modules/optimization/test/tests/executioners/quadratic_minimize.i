[StochasticTools]
[]

[FormFunction]
  type = QuadraticMinimize
  initial_condition = '5 8 1'
  optimization_results = results
  objective = 1.0
  solution = '1 2 3'
[]

[VectorPostprocessors]
  [results]
    type = OptimizationResults
  []
[]

[Executioner]
  type = Optimize
  petsc_options_iname = '-tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type'
  petsc_options_value = '0 1e16 constant'
  solve_on = none
[]

[Outputs]
  csv = true
[]
