[StochasticTools]
[]

[FormFunction]
  type = QuadraticMinimize
  parameter_vpp = 'results'
  measured_vpp = 'measurements'
  objective = 1.0
[]

[VectorPostprocessors]
  [results]
    type = OptimizationParameterVectorPostprocessor
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
  tao_solver = TAOCG
  solve_on = none
  verbose = true
[]

[Outputs]
  csv = true
[]
