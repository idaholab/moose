[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/test]
  type = MooseMeshUtilsTest
[]

[Executioner]
  type = Steady
[]
