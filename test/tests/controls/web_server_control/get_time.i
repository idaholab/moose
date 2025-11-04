# This should be ran by get_postprocessor.py to test
# getting a changing postprocessor value by the server

[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Controls/web_server]
  type = WebServerControl
  execute_on = 'INITIAL TIMESTEP_END'
  initial_client_timeout = 5
  client_timeout = 5
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.123
[]

[Postprocessors]
  [dt]
    type = TimestepSize
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [t]
    type = FunctionValuePostprocessor
    function = 't'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Outputs]
  csv = true
[]
