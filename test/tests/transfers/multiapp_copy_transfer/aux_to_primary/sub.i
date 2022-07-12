[Problem]
  type = FEProblem
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables/sub]
  initial_condition = 1980
[]

[Executioner]
  type = Transient
[]

[Outputs]
  execute_on = 'FINAL'
  exodus = true
[]
