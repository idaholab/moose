[Mesh]
  [iga_file]
    type = FileMeshGenerator
    file = coreform_geom_8.e
    constraint_matrix = geom_8_extraction_op.m
  []
  [sidesets]
    type = SideSetsFromNormalsGenerator
    input = iga_file
    normals = '-1 0 0
               1 0 0'
    fixed_normal = true
    new_boundary = 'left right'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
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
    penalty = 1e9
    variable = u
    boundary = 'left'
    value = 0
  []

  [right]
    type = PenaltyDirichletBC
    penalty = 1e9
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Steady

  # Preconditioned norms to avoid penalty-confused linear "convergence"
  petsc_options_iname = '-ksp_norm_type'
  petsc_options_value = 'preconditioned'

  # A much tighter nonlinear tolerance to avoid penalty-confused
  # nonlinear "convergence"
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-50
[]

[Outputs]
  exodus = true
[]
