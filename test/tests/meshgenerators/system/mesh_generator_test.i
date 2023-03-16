[Mesh]
  active = 'test'
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
  [test]
    type = TestMeshGenerator
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
