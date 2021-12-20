[Problem]
  type = FEProblem
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[AuxVariables/x1]
  initial_condition = 10
[]

[Executioner]
  type = Transient
[]

[Outputs]
  execute_on = 'FINAL'
  exodus = true
[]
