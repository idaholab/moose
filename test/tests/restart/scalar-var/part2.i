[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = part1_out.e
    use_for_exodus_restart = true
  []
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_from_file_var = v
  []
  [lambda]
    family = SCALAR
    order = FIRST
    initial_from_file_var = lambda
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
    phi0 = 1
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-snes_max_it'
  petsc_options_value = '0'
  nl_abs_tol = 1e-10
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
[]
