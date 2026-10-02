[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
[]

[ActionComponents]
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10

    fp = fp
    A = 1.0
    initial_p = 1e5
    initial_T = 300
    initial_vel = 1
    scaling_factor_1phase = '1 1 1e-5'
  []
[]
