# Tests RealToBoolChainControl
#
# The following test function is used:
#   test_fn:  y(t) = { 1   t = 0
#                    { 1   t = 1
#                    { 0   t = 2
#                    { 1   t = 3
#
# Note controls execute BEFORE post-processors, except for INITIAL, so here
# the initial PP value is incorrect, but the others are correct:
#   INITIAL:
#     bool_value <- bool_ctrl:value = 0
#     real_ctrl:value <- y(0) = 1
#     bool_ctrl:value <- bool(real_ctrl:value) = true
#   TIMESTEP_END (t = 1):
#     real_ctrl:value <- y(1) = 1
#     bool_ctrl:value <- bool(real_ctrl:value) = true
#     bool_value <- bool_ctrl:value = 1
#   TIMESTEP_END (t = 2):
#     real_ctrl:value <- y(2) = 0
#     bool_ctrl:value <- bool(real_ctrl:value) = false
#     bool_value <- bool_ctrl:value = 0
#   TIMESTEP_END (t = 3):
#     real_ctrl:value <- y(3) = 1
#     bool_ctrl:value <- bool(real_ctrl:value) = true
#     bool_value <- bool_ctrl:value = 1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [test_fn]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '1 1 0 1'
  []
[]

[ChainControls]
  [real_ctrl]
    type = GetFunctionValueChainControl
    function = test_fn
    point = '0 0 0'
  []
  [bool_ctrl]
    type = RealToBoolChainControl
    input = real_ctrl:value
  []
[]

[Postprocessors]
  [bool_value]
    type = ChainControlDataPostprocessor
    chain_control_data_name = bool_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
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
