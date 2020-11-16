[Mesh]
  [./dmg]
    type = DistributedRectilinearMeshGenerator
    nx = 10
    ny = 10
    dim = 2
  [../]
[]

[Variables]
  [u1][]
  [u2][]
  [u3][]
[]

[Kernels]
  [./diff_1]
    type = Diffusion
    variable = u1
  [../]
  [./diff_2]
    type = Diffusion
    variable = u2
  [../]
  [./diff_3]
    type = Diffusion
    variable = u3
  [../]
[]

[BCs]
  [./left_1]
    type = DirichletBC
    variable = u1
    boundary = 'left'
    value = 0
  [../]

  [./right_1]
    type = DirichletBC
    variable = u1
    boundary = 'right'
    value = 1
  [../]

  [./left_2]
    type = DirichletBC
    variable = u2
    boundary = 'left'
    value = 0
  [../]

  [./right_2]
    type = DirichletBC
    variable = u2
    boundary = 'right'
    value = 2
  [../]

  [./left_3]
    type = DirichletBC
    variable = u3
    boundary = 'left'
    value = 0
  [../]

  [./right_3]
    type = DirichletBC
    variable = u3
    boundary = 'right'
    value = 3
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hmg_use_subspace_coarsening'
  petsc_options_value = 'hmg true'
  petsc_options = '-snes_view'
[]

[Outputs]
  exodus = true
[]
