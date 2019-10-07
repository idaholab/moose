# Tests the derivatives of the density material property for the 7-equation model.

[JacobianTest2Phase]
  A = 1e2
  alpha_vapor = 0.3
  p_liquid = 1e6
  p_vapor = 2e6
  T_liquid = 300
  T_vapor = 500
  vel_liquid = 2
  vel_vapor = 4
  snes_test_err = 1e-12
  fp_2phase = fp_2phase
[]

[FluidProperties]
  [./fp_2phase]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Kernels]
  [./test_kernel]
    type = MaterialDerivativeTestKernel
    variable = beta
    material_property = rho_liquid
    args = 'beta arhoA_liquid'
  [../]
[]
