[FluidPropertiesInterrogator]
  fp = fp
  p = 1e5
  T = 300
  vel = 10
[]

[Modules]
  [./FluidProperties]
    [./fp]
      type = SodiumProperties
    [../]
  [../]
[]
