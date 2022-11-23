[FluidPropertiesInterrogator]
  fp = fp
  rho = 1
  p = 1e5
[]

[FluidProperties]
  [./fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02900055737704918
    mu = 1.823e-05
    k = 0.02568
  [../]
[]
