[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [u]
    initial_condition = 1.0
  []
[]

[Postprocessors]
  [diff]
    type = ElementL2Difference
    variable = u
    other_variable = u
  []
[]

[Executioner]
  type = Steady
[]
