[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [change]
    type = SteadyStateRelativeChangeNorm
  []
[]
