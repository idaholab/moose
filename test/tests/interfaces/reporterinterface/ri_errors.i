[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [error_test]
    type = ReporterInterfaceErrorTest
    reporter = dummy/value
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
