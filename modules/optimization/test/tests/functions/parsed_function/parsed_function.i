[Optimization]
[]

[Reporters]
  [params]
    type = ConstantReporter
    real_vector_names = 'vals'
    # Random numbers (0,1)
    real_vector_values = '0.2045493861327407 0.1683865411251007 0.5071506764525194 0.7257355264179883'
  []
[]

[Functions]
  [parsed_opt]
    type = ParsedOptimizationFunction
    expression = 'x*v1 + y*v2*v2 + z*v3*v3*v3 + t*v4*v4*v4*v4'
    param_symbol_names = 'v1 v2 v3 v4'
    param_vector_name = 'params/vals'
  []
[]

[VectorPostprocessors]
  [test]
    type = OptimizationFunctionTest
    functions = parsed_opt
    # Random numbers (0,1)
    points = '0.1004222428676613 0.21941135757659 0.9310663354302137'
    times = '0.3313593793207535'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
