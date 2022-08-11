[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/test]
  type = ExecFlagRegistryErrorTest
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
