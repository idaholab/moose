[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [other_uo]
    type = UserObjectInterfaceErrorTest
  []
  [error_test]
    type = UserObjectInterfaceErrorTest
    uo = other_uo
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
