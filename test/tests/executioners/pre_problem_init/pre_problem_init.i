[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Debug]
  show_actions = true
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = TestSteady
[]
