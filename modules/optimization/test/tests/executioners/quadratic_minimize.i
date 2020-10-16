[StochasticTools]
[]

[FormFunction]
  type = QuadraticMinimize
  initial_condition = '5 8 1'
  optimization_results = results
  optimization_vpp = fixme_lynn_placeholder
  objective = 1.0
  solution = '1 2 3'
[]

[VectorPostprocessors]
  [results]
    type = OptimizationResults
  []
  [fixme_lynn_placeholder]
    type = OptimizationVectorPostprocessor
    parameters = 'lynn1 lynn2 lynn3'
    intial_values = '1 2 3'
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
