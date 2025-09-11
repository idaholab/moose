# This should be ran by get_postprocessor.py to test
# getting a changing postprocessor value by the server

[Mesh]

  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Controls]

  [web_server]
    type = WebServerControl
    execute_on = 'TIMESTEP_BEGIN'
    port = 40000
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 100
  dt = 0.5
[]

[Postprocessors]
  [t]
    type = FunctionValuePostprocessor
    function = 't'
  []

  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  csv = true
[]
