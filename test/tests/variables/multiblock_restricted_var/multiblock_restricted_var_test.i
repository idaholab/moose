[Mesh]
  file = cake_layers.e
[]

[Variables]
  [./v1]
    block = 2
  [../]
  [./v2]
    block = 4
  [../]
  [./w]
  [../]
[]

[Kernels]
  [./diff_v1]
    type = Diffusion
    variable = v1
    block = 2
  [../]
  [./diff_v2]
    type = Diffusion
    variable = v2
    block = 4
  [../]
  [./diff_w]
    type = Diffusion
    variable = w
  [../]
[]

[BCs]
  [./left_w]
    type = DirichletBC
    variable = w
    boundary = left
    value = 0
  [../]
  [./right_w]
    type = DirichletBC
    variable = w
    boundary = right
    value = 1
  [../]
  [./left_v1]
    type = DirichletBC
    variable = v1
    boundary = left_bottom
    value = 0
  [../]
  [./right_v1]
    type = DirichletBC
    variable = v1
    boundary = right_bottom
    value = 1
  [../]
  [./left_v2]
    type = DirichletBC
    variable = v2
    boundary = left_top
    value = 0
  [../]
  [./right_v2]
    type = DirichletBC
    variable = v2
    boundary = right_top
    value = 1
  [../]
[]

[Preconditioning]
  [./smp_full]
    type = SMP
    full = true
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
