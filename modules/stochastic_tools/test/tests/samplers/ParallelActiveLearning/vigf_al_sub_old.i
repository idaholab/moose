#* This sub-app has no real physics -- it exists purely so the master file's
#* SamplerFullSolveMultiApp/MultiAppSamplerControl machinery has something to
#* drive. It evaluates the toy function
#*
#*     f(x) = sin(x) + 2*x
#*
#* at a single controllable value x_input, and reports the result via the
#* Postprocessor "average" (matching the master file's
#* from_reporter = 'average/value').
#*
#* x_input is a ConstantPostprocessor. CONFIRMED from a real run:
#* MultiAppSamplerControl overrides it via a plain CLI_ARGS-style override
#* using the sub-app's full parameter path (Postprocessors/x_input/value),
#* not a control-tag lookup by block name -- see param_names in vigf_al.i.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [x_input]
    type = ConstantPostprocessor
    value = 0.0
  []
  [average]
    type = FunctionValuePostprocessor
    function = true_function
    execute_on = 'initial timestep_end'
  []
[]

[Functions]
  [true_function]
    type = ParsedFunction
    expression = 'sin(x_in) + 2*x_in'
    symbol_names = 'x_in'
    symbol_values = 'x_input'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
[]
