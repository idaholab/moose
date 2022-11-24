[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Components]
  [cmp]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 100
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
