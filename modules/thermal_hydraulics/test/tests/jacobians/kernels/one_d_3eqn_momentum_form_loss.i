[JacobianTestGeneral]
  variable_names = 'arhoA arhouA arhoEA'
  variable_values = '3 4 5'
  aux_variable_names = 'A'
  aux_variable_values = '1'
  snes_test_err = 1e-8
[]

[Materials]
  [rho_mat]
    type = LinearTestMaterial
    name = rho
    symbol_names = 'arhoA'
    slopes = '4'
  []
  [vel_mat]
    type = LinearTestMaterial
    name = vel
    symbol_names = 'arhoA arhouA'
    slopes = '5 6'
  []
  [K_prime_mat]
    type = GenericFunctionMaterial
    prop_names = 'K_prime'
    prop_values = 'K_prime_fn'
  []
[]

[Functions]
  [K_prime_fn]
    type = ConstantFunction
    value = 1
  []
[]

[Kernels]
  [test_kernel]
    type = OneD3EqnMomentumFormLoss
    variable = arhouA
    arhoA = arhoA
    arhouA = arhouA
    arhoEA = arhoEA
    A = A
    rho = rho
    vel = vel
  []
[]
