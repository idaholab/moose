[Mesh]
  file = square.e
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
    penalty = 1e5
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = PenaltyDirichletBC
    penalty = 1e5
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_type -ksp_pc_side'
  petsc_options_value = 'hypre boomeramg cg left'
[]

[Outputs]
  file_base = out
  exodus = true
[]

