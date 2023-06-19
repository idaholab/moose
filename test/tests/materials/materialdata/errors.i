[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/test]
  type = MaterialErrorTest
[]

[Problem]
  solve = False
[]

[Executioner]
  type = Steady
[]
