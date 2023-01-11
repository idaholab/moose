# This test tests the derivatives of the dynamic viscosity material

[JacobianTestGeneral]
  variable_names = 'beta arhoA arhouA arhoEA'
  variable_values = '0.5 1.2 -0.5 3.2'
  snes_test_err = 1e-1
[]

[FluidProperties]
  [fp_1phase]
    type = LinearTestFluidProperties
  []
[]

[Materials]
  [v_material]
    type = LinearTestMaterial
    vars = 'beta arhoA'
    slopes = '0.2 0.7'
    name = v
  []
  [e_material]
    type = LinearTestMaterial
    vars = 'arhoA arhouA arhoEA'
    slopes = '0.3 1.4 2.1'
    name = e
  []
  [mu_material]
    type = DynamicViscosityMaterial
    mu = mu
    beta = beta
    arhoA = arhoA
    arhouA = arhouA
    arhoEA = arhoEA
    fp_1phase = fp_1phase
    v = v
    e = e
  []
[]

[Kernels]
  [test_kernel]
    type = MaterialDerivativeTestKernel
    variable = beta
    material_property = mu
    coupled_variables = 'beta arhoA arhouA arhoEA'
  []
[]
