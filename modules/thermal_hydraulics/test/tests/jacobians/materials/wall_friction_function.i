# Tests the derivatives of WallFrictionFunctionMaterial

[JacobianTestGeneral]
  variable_names = 'beta arhoA arhouA arhoEA'
  variable_values = '2 3 4 5'
  snes_test_err = 1e-1
[]

[Materials]
  [f_D_mat]
    type = WallFrictionFunctionMaterial
    f_D = f_D
    function = 0.01
    beta = beta
    arhoA  = arhoA
    arhouA = arhouA
    arhoEA = arhoEA
  []
[]

[Kernels]
  [test_kernel]
    type = MaterialDerivativeTestKernel
    variable = beta
    material_property = f_D
    coupled_variables = 'beta arhoA arhouA arhoEA'
  []
[]
