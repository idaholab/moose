# Simple run that does no solve and just has a web server waiting
# on the flag INITIAL

[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Controls/web_server]
  type = WebServerControl
  execute_on = 'INITIAL'
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
