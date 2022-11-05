# This test tests the derivatives of the linear test material, which computes a
# fictitious property that is linear with respect to the solution variables.

[JacobianTestGeneral]
  variable_names = 'beta arhoA arhouA arhoEA'
  variable_values = '2 3 4 5'
  snes_test_err = 1e-1
[]

[Materials]
  [linear_test_material]
    type = LinearTestMaterial
    vars = 'beta arhoA arhouA arhoEA'
    slopes = '2.5 3.5 4.5 5.5'
    name = test_property
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
