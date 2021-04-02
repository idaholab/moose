[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [null_uo]
    type = NullUserObject
  []
  [error_test]
    type = UserObjectInterfaceErrorTest
    uo = null_uo
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
