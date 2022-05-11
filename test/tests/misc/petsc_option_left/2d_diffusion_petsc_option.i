[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  petsc_options = "-options_left"
  petsc_options_iname = "-pc_type"
  petsc_options_value = "hypre"
[]
