# Tests GetPostprocessorChainControl
#
# The following test function is used:
#    f(t) = t + 5
# Note controls execute BEFORE post-processors, except for INITIAL, so a lag
# is present as follows:
#   INITIAL:
#     function_pp <- 5
#     test_pp <- get_pp_ctrl = 0
#     get_pp_ctrl <- function_pp = 5
#   First TIMESTEP_END:
#     function_pp <- 6
#     test_pp <- get_pp_ctrl = 5
#     get_pp_ctrl <- function_pp = 6

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
  [test_pp]
    type = ChainControlDataPostprocessor
    chain_control_data_name = function_pp
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ChainControls]
  [get_pp_ctrl]
    type = GetPostprocessorChainControl
    postprocessor = function_pp
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
