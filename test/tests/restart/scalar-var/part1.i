[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 21
[]

[Variables]
  [v]
    type = MooseVariableFVReal
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
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]
