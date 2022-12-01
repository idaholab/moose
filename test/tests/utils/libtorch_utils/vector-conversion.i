[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[VectorPostprocessors]
  [test]
    type = LibtorchVectorConversionTest
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
