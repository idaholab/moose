[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 21
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
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
    type = FVScalarLagrangeMultiplier
    variable = v
    lambda = lambda
    constraint_type = point-value
    point = '0.1 0 0'
    phi0 = 0.5
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
