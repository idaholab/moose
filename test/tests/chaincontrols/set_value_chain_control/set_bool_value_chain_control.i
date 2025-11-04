# Tests SetBoolValueChainControl
#
# The "enable" parameter of the PP "test_pp" will be controlled to disable it.
# When a PP is disabled, it will retain its last executed value.
#
# The following test function is used:
#    f(t) = t + 5
# Note controls execute BEFORE post-processors, except for INITIAL, so here
# the initial PP value is incorrect, but the others are correct:
#   INITIAL:
#     test_pp <- f(0) = 5
#     set_value_ctrl: test_pp:enable <- false
#   TIMESTEP_END (t = 1):
#     set_value_ctrl: test_pp:enable <- false
#     test_pp (no update to value since disabled; keep value of 5)

!include base.i

[ChainControls]
  [false_ctrl]
    type = ConstantBoolChainControl
    value = false
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [set_value_ctrl]
    type = SetBoolValueChainControl
    parameter = Postprocessors/test_pp/enable
    value = false_ctrl:value
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Postprocessors]
  [test_pp]
    type = FunctionValuePostprocessor
    function = test_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
