[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[UserObjects]
  [parse_tester]
    type = ReadVectorValue
    vector_realvv = '0.1 0.2 0.3 0.4 0.5 0.6'
  []
[]

[Executioner]
  type = Steady
[]
