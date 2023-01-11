# Tests the derivatives of the constant material, which all should be zero

[JacobianTestGeneral]
  variable_names = 'beta arhoA arhouA arhoEA'
  variable_values = '2 3 4 5'
  snes_test_err = 1e-1
[]

[Materials]
  [test_material]
    type = ConstantMaterial
    property_name = test_property
    value = 5
    derivative_vars = 'beta arhoA arhouA arhoEA'
  []
[]

[Kernels]
  [test_kernel]
    type = MaterialDerivativeTestKernel
    variable = beta
    material_property = test_property
    coupled_variables = 'beta arhoA arhouA arhoEA'
  []
[]
