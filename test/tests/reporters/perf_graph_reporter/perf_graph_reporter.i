[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters/perf_graph]
  type = PerfGraphReporter
[]

[Outputs]
  json = true
[]
