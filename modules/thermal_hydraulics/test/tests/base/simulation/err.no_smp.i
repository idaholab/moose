[GlobalParams]
  gravity_vector = '0 0 9.81'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  fp = water

  closures = simple_closures
  f = 0
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
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = 1
    T = 300
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = '1'
    A = 1
    D_h = 1
    n_elems = 2
  []

  [jct1]
    type = VolumeJunction1Phase
    position = '1 0 0'
    volume = 1e-3
    connections = 'pipe1:out pipe2:in'
    use_scalar_variables = false
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = '1'
    A = 1
    D_h = 1
    n_elems = 2
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 101325
  []
[]

[Executioner]
  type = Transient

  dt = 0.01
  num_steps = 2
[]
