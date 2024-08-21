[FluidPropertiesInterrogator]
  fp = fp_2phase_ncg
  p = 1e5
  T = 372.7559289
[]

[FluidProperties]
  [fp_nitrogen]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02867055103448276
  []
  [fp_2phase_ncg]
    type = TestTwoPhaseNCGFluidProperties
    fp_ncgs = 'fp_nitrogen'
  []

  [fp_2phase]
    type = StiffenedGasTwoPhaseFluidProperties
  []
  [fp_2phase_ncg_partial_pressure]
    type = TwoPhaseNCGPartialPressureFluidProperties
    fp_2phase = fp_2phase
    fp_ncg = fp_nitrogen
  []
[]
