[JacobianTest1Phase]
  A = 1e-1
  p = 1e5
  T = 300
  vel = 2
  snes_test_err = 1e-6
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
    type = OneDEnergyStagnationPandTBC
    variable = rhoEA
    boundary = 0
    normal = -1
    fp = fp_1phase
    T0 = 300
    p0 = 2e5
    A = A
    vel = vel
    arhoA = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
  []
  [bc_2]
    type = OneDEnergyStagnationPandTBC
    variable = rhoEA
    boundary = 1
    normal = 1
    fp = fp_1phase
    T0 = 300
    p0 = 0.5e5
    A = A
    vel = vel
    arhoA = rhoA
    arhouA = rhouA
    arhoEA = rhoEA
  []
[]
