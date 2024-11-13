# Tests GetPostprocessorChainControl
#
# The following test function is used:
#    f(t) = t + 5
# Note controls execute BEFORE post-processors, except for INITIAL, so a lag
# is present as follows:
#   INITIAL:
#     function_pp <- f(t) = 5
#     test_pp1(2) <- get_pp_ctrl1(2) = 0
#     get_pp_ctrl1(2) <- function_pp = 5
#   First TIMESTEP_END:
#     get_pp_ctrl1(2) <- function_pp = 5
#     function_pp <- f(t) = 6
#     test_pp1(2) <- get_pp_ctrl1(2) = 5
#

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
    expression = 't + 5'
  []
[]

[Postprocessors]
  [function_pp]
    type = FunctionValuePostprocessor
    function = test_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [test_pp1]
    type = ChainControlDataPostprocessor
    chain_control_data_name = function_pp
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [test_pp2]
    type = ChainControlDataPostprocessor
    chain_control_data_name = get_pp_ctrl2:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ChainControls]
  [get_pp_ctrl1]
    type = GetPostprocessorChainControl
    postprocessor = function_pp
    name_data_same_as_postprocessor = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [get_pp_ctrl2]
    type = GetPostprocessorChainControl
    postprocessor = function_pp
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
