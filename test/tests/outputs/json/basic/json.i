[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Reporters]
  [test]
    type = TestDeclareReporter
  []
[]

[Outputs]
  json = true
[]
