[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  elem_type = QUAD8
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 0.0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1.0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
