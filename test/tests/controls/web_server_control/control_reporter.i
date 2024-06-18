# This should be called via control_reporter.py to test
# controlling a parameter in Reporters/test, which is then
# output via JSON to verify that it was changed. It supports
# multiple times depending on which parameters are passed to
# Reporters/test

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
