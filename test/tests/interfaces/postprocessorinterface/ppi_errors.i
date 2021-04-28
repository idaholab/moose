[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/error_test]
  type = PostprocessorInterfaceErrorTest
  pps = '0 1'
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
