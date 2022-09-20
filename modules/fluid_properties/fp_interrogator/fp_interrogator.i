# The parameters in this block are used to specify the thermodynamic state
# at which to query the fluid properties package
[FluidPropertiesInterrogator]
  fp = fp
  p = 1e5
  T = 300
  vel = 10
[]

# The fluid properties (equation of state) to query is defined here
[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]
