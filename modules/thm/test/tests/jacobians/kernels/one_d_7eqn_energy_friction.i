[JacobianTestGeneral]
  variable_names = 'beta arhoA arhouA arhoEA'
  variable_values = '2 3 4 5'
  aux_variable_names = 'A'
  aux_variable_values = '1.5'
  mat_property_names = 'D_h'
  mat_property_values = '2.5'
  snes_test_err = 1e-8
[]

[Materials]
  [./f_D_mat]
    type = LinearTestMaterial
    name = f_D
    vars = 'beta arhoA arhouA arhoEA'
    slopes = '1.5 2.5 3.5 4.5'
  [../]
  [./alpha_mat]
    type = LinearTestMaterial
    name = alpha
    vars = 'beta'
    slopes = '1.6'
  [../]
  [./rho_mat]
    type = LinearTestMaterial
    name = rho
    vars = 'beta arhoA'
    slopes = '1.2 2.3'
  [../]
  [./vel_mat]
    type = LinearTestMaterial
    name = vel
    vars = 'arhoA arhouA'
    slopes = '2 3'
  [../]
  [./f_2phase_mult_mat]
    type = LinearTestMaterial
    name = f_2phase_mult
    vars = 'beta arhoA arhouA arhoEA'
    slopes = '4 5 6 7'
  [../]
[]

[Kernels]
  [./test_kernel]
    type = OneD7EqnEnergyFriction
    variable = arhoEA
    A = A
    D_h = D_h
    beta = beta
    arhoA = arhoA
    arhouA = arhouA
    arhoEA = arhoEA
    alpha = alpha
    rho = rho
    vel = vel
    f_D = f_D
    2phase_multiplier = f_2phase_mult
  [../]
[]
