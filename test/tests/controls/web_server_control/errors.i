[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Controls/web_server]
  type = WebServerControl
  port = 8000 # will get overridden by the script to find an available port
  execute_on = 'INITIAL'
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
