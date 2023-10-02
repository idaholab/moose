[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/test]
  type = LateRestartLoadTest
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
