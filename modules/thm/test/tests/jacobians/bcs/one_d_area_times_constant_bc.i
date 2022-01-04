[JacobianTest1Phase]
  A = 1e2
  p = 1e6
  T = 300
  vel = 2
  snes_test_err = 1e-7
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
  [bc_1_L]
    type = OneDAreaTimesConstantBC
    variable = rhoA
    boundary = 0
    normal = -1
    value = 999
    A = A
  []

  [bc_2_L]
    type = OneDAreaTimesConstantBC
    variable = rhoA
    boundary = 1
    normal = 1
    value = 999
    A = A
  []
[]
