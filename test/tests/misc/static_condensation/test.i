[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  second_order = true
[]

[Variables]
  [u]
    order = SECOND
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = PenaltyDirichletBC
    variable = u
    boundary = left
    value = 0
    penalty = 1e8
  []
  [right]
    type = PenaltyDirichletBC
    variable = u
    boundary = right
    value = 1
    penalty = 1e8
  []
[]

[Preconditioning]
  [sc]
    type = StaticCondensation
    petsc_options_iname = '-ksp_norm_type'
    petsc_options_value = 'preconditioned'
    petsc_options = '-ksp_monitor'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_type'
  petsc_options_value = 'preonly'
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
