[JacobianTest1Phase]
  A = 1e-1
  p = 1e6
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

[Variables]
  [lm]
    family = SCALAR
    order = FIRST
  []
[]

[ScalarKernels]
  [td_lm]
    type = ODETimeDerivative
    variable = lm
  []
[]

[BCs]
  [bc_1]
    type = OneDEqualValueConstraintBC
    variable = rhouA
    boundary = 0
    lambda = lm
    component = 0
    vg = 1
  []
  [bc_2]
    type = OneDEqualValueConstraintBC
    variable = rhouA
    boundary = 1
    lambda = lm
    component = 0
    vg = -1
  []
[]
