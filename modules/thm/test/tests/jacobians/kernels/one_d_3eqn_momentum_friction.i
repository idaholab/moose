[JacobianTestGeneral]
  variable_names = 'rhoA rhouA rhoEA'
  variable_values = '3 4 5'
  aux_variable_names = 'A'
  aux_variable_values = '1.5'
  mat_property_names = 'D_h'
  mat_property_values = '2.5'
  snes_test_err = 1e-8
[]

[Materials]
  [f_D_mat]
    type = LinearTestMaterial
    name = f_D
    vars = 'rhoA rhouA rhoEA'
    slopes = '2.5 3.5 4.5'
  []
  [rho_mat]
    type = LinearTestMaterial
    name = rho
    vars = 'rhoA'
    slopes = '2.3'
  []
  [vel_mat]
    type = LinearTestMaterial
    name = vel
    vars = 'rhoA rhouA'
    slopes = '2 3'
  []
[]

[Kernels]
  [test_kernel]
    type = OneD3EqnMomentumFriction
    variable = rhouA
    A = A
    D_h = D_h
    arhoA = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
    rho = rho
    vel = vel
    f_D = f_D
  []
[]
