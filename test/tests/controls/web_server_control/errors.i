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
