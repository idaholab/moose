[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Problem]
  type = DummyExternalProblem
[]
