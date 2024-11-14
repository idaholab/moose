# Tests GetFunctionValueChainControl
#
# The following test function is used:
#    f(t) = t + 5
# Note controls execute BEFORE post-processors, except for INITIAL, so here
# the initial PP value is incorrect, but the others are correct:
#   INITIAL:
#     test_pp <- get_fn_ctrl:value = 0
#     get_fn_ctrl:value <- f(0) = 5
#   First TIMESTEP_END:
#     get_fn_ctrl:value <- f(1) = 6
#     test_pp <- get_fn_ctrl:value = 6

[Mesh]
  [gen_mg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[Functions]
  [test_fn]
    type = ParsedFunction
    expression = 'x * (t + 5)'
  []
[]

[Postprocessors]
  [test_pp]
    type = ChainControlDataPostprocessor
    chain_control_data_name = get_fn_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ChainControls]
  [get_fn_ctrl]
    type = GetFunctionValueChainControl
    function = test_fn
    point = '1 0 0'
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
