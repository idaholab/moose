[StochasticTools]
[]

[FormFunction]
  type = QuadraticMinimize
  optimization_vpp = 'results'
  measurement_vpp = 'measurements'
  objective = 1.0
[]

[VectorPostprocessors]
  [results]
    type = OptimizationVectorPostprocessor
    parameters = 'param_0 param_1 param_2'
    intial_values = '5 8 1'
  []
  [measurements]
    type = ConstantVectorPostprocessor
    value = '1 2 3'
    outputs = none
    vector_names = 'values'
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
