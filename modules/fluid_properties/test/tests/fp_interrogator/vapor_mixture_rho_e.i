[FluidPropertiesInterrogator]
  fp = fp_vapor_mix
  rho = 1.1870052372064208
  e = 2477165.9033225174
  x_ncg = '0.1'
[]

[FluidProperties]
  [./fp_nitrogen]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02867055103448276
  [../]
  [./fp_primary]
    type = IdealGasFluidProperties
    gamma = 1.3
    molar_mass = 0.027714866
    T_c = 126.19
    rho_c = 313.189812
  [../]
  [./fp_vapor_mix]
    type = IdealRealGasMixtureFluidProperties
    fp_primary = fp_primary
    fp_secondary = 'fp_nitrogen'
  [../]
[]
