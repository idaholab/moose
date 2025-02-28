[FluidPropertiesInterrogator]
  fp = fp_2phase_ncg_partial_pressure
  p = 1e5
  T = 372.7559289
[]

[FluidProperties]
  [fp_water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
    M = 0.01801488
  []
  [fp_steam]
    type = IdealGasFluidProperties
    gamma = 1.43
    molar_mass = 0.01801488
  []
  [fp_2phase]
    type = TestTwoPhaseFluidProperties
    fp_liquid = fp_water
    fp_vapor = fp_steam
  []

  [fp_air]
    type = IdealGasFluidProperties
  []

  [fp_2phase_ncg_partial_pressure]
    type = TwoPhaseNCGPartialPressureFluidProperties
    fp_2phase = fp_2phase
    fp_ncg = fp_air
  []
[]
