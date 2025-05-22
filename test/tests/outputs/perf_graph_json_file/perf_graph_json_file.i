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

# To make sure this doesn't show up in output
[Reporters/constant]
  type = ConstantReporter
  real_names = value
  real_values = 1
[]

[Outputs]
  perf_graph_json_file = pg.json
  json = true
[]
