# This should be ran by get_postprocessor.py to test
# getting a changing postprocessor value by the server

[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Controls/web_server]
  type = WebServerControl
  execute_on = 'TIMESTEP_END'
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.123
[]

[Postprocessors/t]
  type = FunctionValuePostprocessor
  function = 't'
  execute_on = 'TIMESTEP_BEGIN'
[]

[Outputs]
  csv = true
[]
