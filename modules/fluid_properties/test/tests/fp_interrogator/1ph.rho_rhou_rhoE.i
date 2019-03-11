[FluidPropertiesInterrogator]
  fp = fp
  rho = 0.5
  rhou = 0.5
  rhoE = 2.75
[]

[Modules]
  [./FluidProperties]
    [./fp]
      type = IdealGasFluidProperties
      gamma = 1.4
      R = 0.71428571428571428571
    [../]
  [../]
[]
