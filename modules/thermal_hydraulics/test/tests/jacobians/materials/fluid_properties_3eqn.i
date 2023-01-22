[JacobianTest1Phase]
  A = 10
  p = 1e5
  T = 300
  vel = 2
  fp_1phase = fp_1phase
[]

[FluidProperties]
  [fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Kernels]
  [test_kernel]
    type = MaterialDerivativeTestKernel
    variable = rhoA
    material_property = <none>
    coupled_variables = 'rhoA rhouA rhoEA'
  []
[]
