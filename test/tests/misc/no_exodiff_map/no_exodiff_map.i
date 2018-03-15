[Mesh]
  type = FileMesh
  file = double_square.e
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left1]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./right1]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 2
  [../]
  [./left2]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 3
  [../]
  [./right2]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 4
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'


  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
