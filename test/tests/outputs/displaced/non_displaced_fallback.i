[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[AuxVariables]
  [./a]
  [../]
[]

[AuxKernels]
  [./a_ak]
    type = ConstantAux
    variable = a
    value = 1.
    execute_on = initial
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [./exodus]
    type = Exodus
    use_displaced = true
  [../]
[]
