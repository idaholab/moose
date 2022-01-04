[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [total_power]
    type = TotalPower
    power = 1
  []

  [fch1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = 1
    n_elems = 2

    A = 1
    f = 0.1
    fp = fp
    closures = simple_closures

    initial_T = 300
    initial_p = 1e05
    initial_vel = 0
  []

  [hs]
    type = HeatSourceVolumetric1Phase
    flow_channel = fch1
    q = 1
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = fch1:in
    m_dot = 1
    T = 300
  []

  [outlet]
    type = Outlet1Phase
    input = fch1:out
    p = 1e-5
  []
[]

[Executioner]
  type = Transient
  dt = 1.e-2
[]
