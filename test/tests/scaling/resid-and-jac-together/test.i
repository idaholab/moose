[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 21
  xmax = 2
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    # singular if we use two term boundary expansion
    two_term_boundary_expansion = false
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[FVKernels]
  [advection]
    type = FVElementalAdvection
    variable = v
    velocity = '1 0 0'
  []
  [lambda]
    type = FVIntegralValueConstraint
    variable = v
    lambda = lambda
    phi0 = 1
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-12
  solve_type = NEWTON
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  verbose = true
[]

[Outputs]
  exodus = true
[]
