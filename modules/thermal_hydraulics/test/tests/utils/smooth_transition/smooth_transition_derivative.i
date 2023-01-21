[JacobianTestGeneral]
  variable_names = 'u'
  variable_values = '0'
  snes_test_err = 1e-8
[]

[Materials]
  [test_mat]
    type = SmoothTransitionTestMaterial
    transition_type = cubic
    var = u
  []
[]

[Kernels]
  [test_kernel]
    type = MaterialDerivativeTestKernel
    variable = u
    material_property = mymatprop
    coupled_variables = ''
  []
[]
