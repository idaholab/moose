[FluidPropertiesInterrogator]
  fp = fp
  p = 1e5
  T = 300
[]

[FluidProperties]
  [./fp_liquid]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02900055737704918
    mu = 1.823e-05
    k = 0.02568
  [../]
  [./fp_vapor]
    type = IdealGasFluidProperties
    gamma = 1.1
    molar_mass = 0.027714866
    mu = 1.7e-05
    k = 0.05
  [../]
  [./fp]
    type = TestTwoPhaseFluidProperties
    fp_liquid = fp_liquid
    fp_vapor = fp_vapor
  [../]
[]
