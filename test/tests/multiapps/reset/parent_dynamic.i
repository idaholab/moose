[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 1
[]

[Functions]
  # Will reset every odd time
  [reset_func]
    type = ParsedFunction
    expression = 'if(int(t) % 2, t, 0)'
  []
[]

[Times]
  [reset_time]
    type = FunctorTimes
    functor = reset_func
    outputs = none
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    execute_on = timestep_end
    input_files = sub_dynamic.i
    reset_apps = 0
    reset_time = reset_time
  []
[]

[Postprocessors]
  [sub_time]
    type = Receiver
  []
  [sub_time_expect]
    type = ParsedPostprocessor
    expression = 'if(int(t) % 2, 1, 2)'
    use_t = true
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppReporterTransfer
    from_multi_app = sub
    from_reporters = 'time/value'
    to_reporters = 'sub_time/value'
  []
[]

[UserObjects]
  # Check that sub-app time is what is expected due to resetting every odd time
  [check]
    type = Terminator
    expression = 'sub_time != sub_time_expect'
    error_level = ERROR
    message = 'Sub-application time does not match expected time.'
    # Ensure this happens after transfer
    execute_on = MULTIAPP_FIXED_POINT_END
  []
[]
