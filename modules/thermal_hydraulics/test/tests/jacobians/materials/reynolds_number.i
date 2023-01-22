[JacobianTestGeneral]
  variable_names = 'beta arhoA arhouA arhoEA'
  variable_values = '2 3 4 5'
  snes_test_err = 1e-8
[]

[Materials]
  [rho_mat]
    type = LinearTestMaterial
    name = rho
    vars = 'beta arhoA'
    slopes = '1.5 0.8'
  []
  [vel_mat]
    type = LinearTestMaterial
    name = vel
    vars = 'arhoA arhouA'
    slopes = '-0.6 -1.1'
  []
  [mu_mat]
    type = LinearTestMaterial
    name = mu
    vars = 'beta arhoA arhouA arhoEA'
    slopes = '0.7 -2.2 0.4 -0.6'
  []
  [D_h_mat]
    type = ConstantMaterial
    property_name = D_h
    value = 0.2
  []
  [Re_mat]
    type = ReynoldsNumberMaterial
    beta = beta
    arhoA = arhoA
    arhouA = arhouA
    arhoEA = arhoEA
    Re = Re
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
  []
[]

[Kernels]
  [test_kernel]
    type = MaterialDerivativeTestKernel
    variable = beta
    material_property = Re
    coupled_variables = 'beta arhoA arhouA arhoEA'
  []
[]
