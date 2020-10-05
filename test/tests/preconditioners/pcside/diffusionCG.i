[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
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
    type = PenaltyDirichletBC
    penalty = 1e9
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = PenaltyDirichletBC
    penalty = 1e9
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_type -ksp_norm_type'
  petsc_options_value = 'hypre boomeramg cg preconditioned'
# We are using preconditioned norm because of PenaltyDirichletBC
[]

[Outputs]
  file_base = out
  exodus = true
[]
