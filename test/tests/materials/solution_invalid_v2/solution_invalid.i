[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/test]
  type = PerfGraphTest
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
