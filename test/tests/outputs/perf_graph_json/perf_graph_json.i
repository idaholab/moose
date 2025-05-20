[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  perf_graph_json = true
[]
