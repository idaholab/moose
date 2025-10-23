# Tests tripping, turning an object on, then turning it back off based on the value of a function
# Note: to keep the object on, use a UnitTripChainControl

!include base.i

[Functions]
  [heaviside]
    type = ParsedFunction
    expression = 'if(t>1.1 & t < 2.1, 1, 0)'
  []
[]

[ChainControls]
  [false_then_true_then_false]
    type = GetFunctionValueChainControl
    function = 'heaviside'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [convert_to_bool]
    type = RealToBoolChainControl
    input = 'false_then_true_then_false:value'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [set_value_ctrl]
    type = SetBoolValueChainControl
    parameter = Postprocessors/test_pp/enable
    value = convert_to_bool:value
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

[Debug]
  show_chain_control_data = true
[]
