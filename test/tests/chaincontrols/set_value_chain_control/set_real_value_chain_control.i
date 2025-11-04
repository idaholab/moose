# Tests SetRealValueChainControl
#
# The following test function is used:
#    f(t) = t + 5
# Note controls execute BEFORE post-processors, except for INITIAL, so here
# the initial PP value is incorrect, but the others are correct:
#   INITIAL:
#     test_pp <- test_pp:value = 0
#     get_fn_ctrl:value <- f(0) = 5
#     set_value_ctrl: test_pp:value <- get_fn_ctrl:value = 5
#   TIMESTEP_END (t = 1):
#     get_fn_ctrl:value <- f(1) = 6
#     set_value_ctrl: test_pp:value <- get_fn_ctrl:value = 6
#     test_pp <- test_pp:value = 6

!include base.i

[ChainControls]
  [get_fn_ctrl]
    type = GetFunctionValueChainControl
    function = test_fn
    point = '1 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [set_value_ctrl]
    type = SetRealValueChainControl
    parameter = Postprocessors/test_pp/value
    value = get_fn_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Postprocessors]
  [test_pp]
    type = ConstantPostprocessor
    value = 0
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

