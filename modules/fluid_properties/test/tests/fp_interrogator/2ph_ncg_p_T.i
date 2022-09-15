[FluidPropertiesInterrogator]
  fp = fp_2phase_ncg
  p = 1e5
  T = 372.7559289
  x_ncg = '0.1'
[]

[FluidProperties]
  [./fp_nitrogen]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02867055103448276
  [../]
  [./fp_2phase_ncg]
    type = TestTwoPhaseNCGFluidProperties
    fp_ncgs = 'fp_nitrogen'
  [../]
[]
