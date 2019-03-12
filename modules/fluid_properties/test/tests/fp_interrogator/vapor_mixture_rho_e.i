[FluidPropertiesInterrogator]
  fp = fp_vapor_mix
  rho = 1.1870052372064208
  e = 2477165.9033225174
  x_ncg = '0.1'
[]

[Modules]
  [./FluidProperties]
    [./fp_nitrogen]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 290
    [../]
    [./fp_primary]
      type = IdealGasFluidProperties
      gamma = 1.3
      R = 300
    [../]
    [./fp_vapor_mix]
      type = GeneralVaporMixtureFluidProperties
      fp_primary = fp_primary
      fp_secondary = 'fp_nitrogen'
    [../]
  [../]
[]
