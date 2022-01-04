[JacobianTest1Phase]
  A = 1e-1
  p = 1e6
  T = 300
  vel = 2
  snes_test_err = 1e-8
  fp_1phase = fp_1phase
[]

[FluidProperties]
  [fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[BCs]
  [bc_1]
    type = OneDMomentumHRhoUBC
    variable = rhouA
    boundary = 0
    normal = -1
    rhou = 123
    A = A
    arhoA  = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
    p = p
  []
  [bc_2]
    type = OneDMomentumHRhoUBC
    variable = rhouA
    boundary = 1
    normal = 1
    rhou = 123
    A = A
    arhoA  = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
    p = p
  []
[]
