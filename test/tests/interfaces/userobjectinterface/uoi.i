[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [other_uo]
    type = UserObjectInterfaceTest
  []
  [test]
    type = UserObjectInterfaceTest
    uo = other_uo
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
