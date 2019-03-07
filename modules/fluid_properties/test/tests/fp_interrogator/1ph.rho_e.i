[FluidPropertiesInterrogator]
  fp = fp
  rho = 1
  e = 2.1502500000e+05
[]

[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 286.7
      mu = 1.823e-05
      k = 0.02568
    [../]
  [../]
[]
