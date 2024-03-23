[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Reporters/test]
  type = WebServerControlTestReporter
  execute_on = 'INITIAL TIMESTEP_END'
[]

[Controls/web_server]
  type = WebServerControl
  port = 8000 # will get overridden by the script to find an available port
  execute_on = TIMESTEP_END
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs/json]
  type = JSON
  execute_on = 'INITIAL TIMESTEP_END'
  execute_system_information_on = NONE
[]
