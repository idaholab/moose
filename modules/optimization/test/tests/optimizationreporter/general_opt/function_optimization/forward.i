[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Functions]
  [obj_func]
    type = ParsedOptimizationFunction
    expression = 'pow(x_val + 2 * y_val - 7, 2) + pow(2 * x_val + y_val - 5, 2)'
    param_symbol_names = 'x_val y_val'
    param_vector_name = vals/vals
  []
  [df_dx]
    type = ParsedOptimizationFunction
    expression = '2 * (x_val + 2 * y_val - 7) + 4 * (2 * x_val + y_val - 5)'
    param_symbol_names = 'x_val y_val'
    param_vector_name = vals/vals
  []
  [df_dy]
    type = ParsedOptimizationFunction
    expression = '4 * (x_val + 2 * y_val - 7) + 2 * (2 * x_val + y_val - 5)'
    param_symbol_names = 'x_val y_val'
    param_vector_name = vals/vals
  []
[]

[Postprocessors]
  [obj_pp]
    type = FunctionValuePostprocessor
    function = obj_func
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = console
  []
  [df_dx]
    type = FunctionValuePostprocessor
    function = df_dx
  []
  [df_dy]
    type = FunctionValuePostprocessor
    function = df_dy
  []

[]

[VectorPostprocessors]
  [grad_f]
    type = VectorOfPostprocessors
    postprocessors = 'df_dx df_dy'
  []
[]

[Reporters]
  [vals]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '0 4'
  []
[]

