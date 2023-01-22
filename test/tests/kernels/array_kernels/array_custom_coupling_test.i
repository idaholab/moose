[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 4
    ny = 4
    dim = 2
  []
[]


[Variables]
  [u]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
  []
[]

[DGKernels]
  [dgdiff]
    type = ArrayDGDiffusion
    variable = u
    diff = dc
  []
[]

[BCs]
  [left]
    type = ArrayVacuumBC
    variable = u
    boundary = 1
  []

  [right]
    type = ArrayPenaltyDirichletBC
    variable = u
    boundary = 2
    value = '1 2'
    penalty = 4
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [rc]
    type = GenericConstant2DArray
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Preconditioning]
  [pbp]
    type = PBP
    solve_order = 'u_0 u_1'
    preconditioner = 'AMG AMG'
    off_diag_row = 'u_0 u_1'
    off_diag_column = 'u_0 u_1'
  []
[]

[Executioner]
  type = Steady
  solve_type = JFNK
  petsc_options = '-mat_view'
[]

[Outputs]
  exodus = true
[]
