[Mesh]
  file = rectangle.e
[]

[Variables]
  [./u]
    block = 1
  [../]
  [./v]
  [../]
[]

[AuxVariables]
  [./w]
  [../]
[]

[Kernels]
  [./diffu]
    type = WrongJacobianDiffusion
    block = 1
    jfactor = 0.9
    variable = u
  [../]
  [./diffv]
    type = WrongJacobianDiffusion
    jfactor = 0.7
    variable = v
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

