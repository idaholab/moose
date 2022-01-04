# Tests that we report an error if users try to connect to a non-existent component

[GlobalParams]
  initial_p = 1e5
  initial_T = 300
  initial_vel = 0
  closures = simple_closures
[]

[FluidProperties]
  [water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    fp = water
    position = '0 0 0'
    orientation = '0 1 0'
    length = 1
    n_elems = 2
    A = 1e-4
    f = 0
  []
  [inlet_1p]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    m_dot = 1
    T = 300
  []
  [outlet_1p]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 1e5
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
[]
