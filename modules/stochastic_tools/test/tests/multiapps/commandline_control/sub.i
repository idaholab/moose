[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
    initial_condition = 1980
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
  [size]
    type = AverageElementSize
    execute_on = 'initial'
  []
[]

[Outputs]
  csv = true
[]
