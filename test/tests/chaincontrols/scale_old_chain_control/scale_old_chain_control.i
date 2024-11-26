# Tests ScaleOldChainControl
#
# The following test function is used:
#   scale_fn:  y(t) = t + 1
#
# Note controls execute BEFORE post-processors, except for INITIAL, so here
# the initial PP value is incorrect, but the others are correct:
#   INITIAL:
#     scale_ctrl_value <- scale_ctrl:value = 1
#     scale_factor_ctrl:value <- y(0) = 1
#     scale_ctrl:value <- 3 * scale_factor_ctrl:value = 3
#   TIMESTEP_END (t = 1):
#     scale_factor_ctrl:value <- y(1) = 2
#     scale_ctrl:value <- 3 * scale_factor_ctrl:value = 6
#     scale_ctrl_value <- scale_ctrl:value = 6
#   TIMESTEP_END (t = 2):
#     scale_factor_ctrl:value <- y(2) = 3
#     scale_ctrl:value <- 6 * scale_factor_ctrl:value = 18
#     scale_ctrl_value <- scale_ctrl:value = 18
#   TIMESTEP_END (t = 3):
#     scale_factor_ctrl:value <- y(1) = 4
#     scale_ctrl:value <- 18 * scale_factor_ctrl:value = 72
#     scale_ctrl_value <- scale_ctrl:value = 72

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [scale_fn]
    type = ParsedFunction
    expression = 't + 1'
  []
[]

[Postprocessors]
  [scale_ctrl_value]
    type = ChainControlDataPostprocessor
    chain_control_data_name = scale_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ChainControls]
  [scale_factor_ctrl]
    type = GetFunctionValueChainControl
    function = scale_fn
    point = '0 0 0'
  []
  [scale_ctrl]
    type = ScaleOldChainControl
    initial_value = 3
    scale_factor = scale_factor_ctrl:value
    control_data = scale_ctrl:value
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
