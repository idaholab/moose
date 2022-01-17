[JacobianTest1Phase]
  A = 1e-1
  p = 1e6
  T = 300
  vel = 2
  snes_test_err = 1e-10
  use_transient_executioner = true
  fp_1phase = fp_1phase
[]

[Modules/FluidProperties]
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
    type = OneDEnergyStaticPressureBC
    variable = rhoEA
    boundary = 0
    rho = rho
    rhoA = rhoA
    rhouA = rhouA
    vel = vel
    e = e
    v = v
    normal = -1
    A = A
    fp = fp_1phase
    p_in = 6e6
  []
  [bc_2]
    type = OneDEnergyStaticPressureBC
    variable = rhoEA
    boundary = 1
    rho = rho
    rhoA = rhoA
    rhouA = rhouA
    vel = vel
    e = e
    v = v
    normal = 1
    A = A
    fp = fp_1phase
    p_in = 6e6
  []
[]
