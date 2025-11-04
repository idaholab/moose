# This tests controllability of the enable parameter of a MOOSE object via a
# set of times

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Times]
  [times_on]
    type = InputTimes
    # 8 should lie within the default window
    times = '0 1 5 8.00000000001 10'
  []
[]

[Controls]
  [times]
    type = TimesEnableControl
    times = times_on
    enable_objects = 'Postprocessors::count_on'
    disable_objects = 'Postprocessors::count_off'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Debug]
  show_execution_order = ALWAYS
[]

[Postprocessors]
  [count_on]
    type = TestPostprocessor
    test_type = 'grow'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [count_off]
    type = TestPostprocessor
    test_type = 'grow'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 10
[]

[Outputs]
  csv = true
[]
