[FluidPropertiesInterrogator]
  fp = fp
  T = 300
[]

[Modules]
  [./FluidProperties]
    [./fp_liquid]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 286.7
      mu = 1.823e-05
      k = 0.02568
    [../]
    [./fp_vapor]
      type = IdealGasFluidProperties
      gamma = 1.1
      R = 300
      mu = 1.7e-05
      k = 0.05
    [../]
    [./fp]
      type = TestTwoPhaseFluidProperties
      fp_liquid = fp_liquid
      fp_vapor = fp_vapor
    [../]
  [../]
[]
