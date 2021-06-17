[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Reporters/error_test]
  type = TestDeclareErrorsReporter
  value = value_name
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
