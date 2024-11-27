# Tests RealToBoolChainControl
#
# Note controls execute BEFORE post-processors, except for INITIAL, so here
# the initial PP value is incorrect, but the others are correct:
#   INITIAL:
#     true_trip <- true_trip_ctrl:value = 0
#     false_trip <- false_trip_ctrl:value = 0
#     true_trip_ctrl:value <- false
#     false_trip_ctrl:value <- false
#   TIMESTEP_END (t = 1):
#     true_trip_ctrl:value <- false
#     false_trip_ctrl:value <- false
#     true_trip <- true_trip_ctrl:value = 0
#     false_trip <- false_trip_ctrl:value = 0
#   TIMESTEP_END (t = 2):
#     true_trip_ctrl:value <- false
#     false_trip_ctrl:value <- true
#     true_trip <- true_trip_ctrl:value = 0
#     false_trip <- false_trip_ctrl:value = 1
#   TIMESTEP_END (t = 3):
#     true_trip_ctrl:value <- true
#     false_trip_ctrl:value <- true
#     true_trip <- true_trip_ctrl:value = 1
#     false_trip <- false_trip_ctrl:value = 1
#   TIMESTEP_END (t = 4):
#     true_trip_ctrl:value <- true
#     false_trip_ctrl:value <- true
#     true_trip <- true_trip_ctrl:value = 1
#     false_trip <- false_trip_ctrl:value = 1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [true_trip_fn]
    type = PiecewiseLinear
    x = '0 1 2 3 4'
    y = '0 0 0 1 0'
  []
  [false_trip_fn]
    type = PiecewiseLinear
    x = '0 1 2 3 4'
    y = '1 1 0 1 1'
  []
[]

[ChainControls]
  [true_trip_fn_ctrl]
    type = GetFunctionValueChainControl
    function = true_trip_fn
  []
  [false_trip_fn_ctrl]
    type = GetFunctionValueChainControl
    function = false_trip_fn
  []
  [true_trip_bool_ctrl]
    type = RealToBoolChainControl
    input = true_trip_fn_ctrl:value
  []
  [false_trip_bool_ctrl]
    type = RealToBoolChainControl
    input = false_trip_fn_ctrl:value
  []
  [true_trip_ctrl]
    type = UnitTripChainControl
    input = true_trip_bool_ctrl:value
    trip_on_true = true
  []
  [false_trip_ctrl]
    type = UnitTripChainControl
    input = false_trip_bool_ctrl:value
    trip_on_true = false
  []
[]

[Postprocessors]
  [true_trip]
    type = ChainControlDataPostprocessor
    chain_control_data_name = true_trip_ctrl:tripped
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [false_trip]
    type = ChainControlDataPostprocessor
    chain_control_data_name = false_trip_ctrl:tripped
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 4
[]

[Outputs]
  csv = true
[]
