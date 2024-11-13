# Tests ParsedChainControl
#
# The following test functions are used:
#   myvar:    u = 4
#   myfn:     f(x,y,z,t) = x * (t + 5)
#   ctrl_fn:  g(x,y,z,t) = y * (t + 2)
#
#   real_ctrl: h(t) = g(x=0,y=1,z=0,t)
#   parsed_ctrl: j(t) = f(x=1,y=2,z=3,t) + u + real_ctrl:value + 1 + 0 + x(=1) + y(=2) + z(=3) + t + 3
#                     = t + 5 + 4 + real_ctrl:value + 1 + 0 + 1 + 2 + 3 + t + 3
#                     = real_ctrl:value + 2*t + 19
#
# Note controls execute BEFORE post-processors, except for INITIAL, so here
# the initial PP value is incorrect, but the others are correct:
#   INITIAL:
#     test_pp <- parsed_ctrl:value = 0
#     real_ctrl:value <- h(0) = 0
#     parsed_ctrl:value <- j(0) = 19
#   TIMESTEP_END (t = 1):
#     real_ctrl:value <- h(1) = 3
#     parsed_ctrl:value <- j(1) = 24
#     test_pp <- parsed_ctrl:value = 24
#   TIMESTEP_END (t = 2):
#     real_ctrl:value <- h(1) = 4
#     parsed_ctrl:value <- j(1) = 27
#     test_pp <- parsed_ctrl:value = 27
#   TIMESTEP_END (t = 3):
#     real_ctrl:value <- h(1) = 5
#     parsed_ctrl:value <- j(1) = 30
#     test_pp <- parsed_ctrl:value = 30

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [myfn]
    type = ParsedFunction
    expression = 'x * (t + 5)'
  []
  [ctrl_fn]
    type = ParsedFunction
    expression = 'y * (t + 2)'
  []
[]

[AuxVariables]
  [myvar]
    family = SCALAR
    order = FIRST
    initial_condition = 4
  []
[]

[Postprocessors]
  [test_pp]
    type = ChainControlDataPostprocessor
    chain_control_data_name = parsed_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ChainControls]
  [real_ctrl]
    type = GetFunctionValueChainControl
    function = ctrl_fn
    point = '0 1 0'
  []
  [true_ctrl]
    type = ConstantBoolChainControl
    value = true
  []
  [false_ctrl]
    type = ConstantBoolChainControl
    value = false
  []
  [parsed_ctrl]
    type = ParsedChainControl
    expression = 'fn + u + realval + trueval + falseval + x + y + z + t + num'
    symbol_names = 'fn u realval trueval falseval num'
    symbol_values = 'myfn myvar real_ctrl:value true_ctrl:value false_ctrl:value 3.0'
    point = '1 2 3'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
[]

[Outputs]
  csv = true
[]
