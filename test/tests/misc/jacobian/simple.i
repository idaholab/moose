[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./s]
  [../]
  [./t]
  [../]
  [./u]
  [../]
  [./u2]
  [../]
  [./v]
  [../]
[]

[AuxVariables]
  [./w]
  [../]
[]

[Kernels]
  [./diffs]
    type = WrongJacobianDiffusion
    variable = s
    jfactor = 0.995
  [../]
  [./difft]
    type = WrongJacobianDiffusion
    variable = t
    jfactor = 2.0
  [../]
  [./diffu]
    type = WrongJacobianDiffusion
    variable = u
    jfactor = 0.0
  [../]
  [./diffu2]
    type = WrongJacobianDiffusion
    variable = u2
    rfactor = 0.0
  [../]
  [./diffv]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'


  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

