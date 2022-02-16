[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 100
[]

[AuxVariables]
  [./min]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./max]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./min]
    type = ElementLengthAux
    variable = min
    method = min
    execute_on = initial
  [../]
  [./max]
    type = ElementLengthAux
    variable = max
    method = max
    execute_on = initial
  [../]
[../]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  exodus = true
[]
