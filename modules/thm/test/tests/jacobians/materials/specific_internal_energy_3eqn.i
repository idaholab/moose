# Tests the derivatives of the specific internal energy material property for the 3-equation model.

[JacobianTest1Phase]
  A = 10
  p = 1e5
  T = 300
  vel = 2
  snes_test_err = 1e-8
  fp_1phase = fp_1phase
[]

[FluidProperties]
  [./fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  [../]
[]

[Kernels]
  [./test_kernel]
    type = MaterialDerivativeTestKernel
    variable = rhoA
    material_property = e
    args = 'rhoA rhouA rhoEA'
  [../]
[]
