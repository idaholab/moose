# Tests tripping a boolean then disabling it on the next time step

!include base.i

[Functions]
  [heaviside]
    type = ParsedFunction
    expression = 'if(t>1.1, 1, 0)'
  []
[]

[ChainControls]
  [false_then_true]
    type = GetFunctionValueChainControl
    function = 'heaviside'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [convert_to_bool]
    type = RealToBoolChainControl
    input = 'false_then_true:value'
  []
  [has_activated]
    type = UnitTripChainControl
    input = convert_to_bool:value
    trip_on_true = true
  []
  [has_activated_old]
    type = UnitTripChainControl
    input = convert_to_bool:value
    trip_on_true = true
    use_old_input = true
  []
  # This chain control lets us perform the flip so the control activates objects for a single time step
  [should_activate]
    type = ParsedChainControl
    expression = 'if(has_activated,if(was_activated, 0, 1), 0)'
    symbol_names = 'has_activated was_activated'
    symbol_values = 'has_activated:tripped has_activated_old:tripped'
  []
  [convert_should_activate_to_bool]
    type = RealToBoolChainControl
    input = 'should_activate:value'
  []
  [set_value_ctrl]
    type = SetBoolValueChainControl
    parameter = Postprocessors/test_pp/enable
    value = convert_should_activate_to_bool:value
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
