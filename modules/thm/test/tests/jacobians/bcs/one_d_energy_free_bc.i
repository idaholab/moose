[JacobianTest1Phase]
  A = 10
  p = 1e5
  T = 300
  vel = 2
  snes_test_err = 1e-9
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
    type = OneDEnergyFreeBC
    variable = rhoEA
    boundary = 0
    normal = -1
    arhoA  = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
    H = H
    vel = vel
    A = A
    p = p
  []
  [bc_2]
    type = OneDEnergyFreeBC
    variable = rhoEA
    boundary = 1
    normal = 1
    arhoA  = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
    H = H
    vel = vel
    A = A
    p = p
  []
[]
