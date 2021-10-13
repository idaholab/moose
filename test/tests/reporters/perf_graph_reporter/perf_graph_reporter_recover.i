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
  type = Transient
  num_steps = 2
[]

[Reporters/perf_graph]
  type = PerfGraphReporter
  execute_on = FINAL
[]

[Outputs/json]
  type = JSON
  execute_on = FINAL
[]
