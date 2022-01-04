[JacobianTest1Phase]
  A = 1e-1
  p = 1e6
  T = 500
  vel = 1000
  fp_1phase = fp
  snes_test_err = 1e-10
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 1.43
    cv = 1040.0
    q = 2.03e6
    p_inf = 0.0
    q_prime = -2.3e4
  []
[]

[BCs]
  [bc_1]
    type = OneDMomentumStaticPressureBC
    variable = rhouA
    boundary = 0
    normal = -1
    rhoA = rhoA
    rhoEA = rhoEA
    A = A
    p_in = 6e6
  []
  [bc_2]
    type = OneDMomentumStaticPressureBC
    variable = rhouA
    boundary = 1
    normal = 1
    rhoA = rhoA
    rhoEA = rhoEA
    A = A
    p_in = 6e6
  []
[]
