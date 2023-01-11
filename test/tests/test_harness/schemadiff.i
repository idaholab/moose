[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [test]
    type = TestDeclareReporter
  []
[]

[Outputs]
  json = true
[]
