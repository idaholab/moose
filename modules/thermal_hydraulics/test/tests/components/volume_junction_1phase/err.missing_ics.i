[GlobalParams]
  gravity_vector = '0 0 0'

  A = 1e-4
  f = 0
  fp = fp

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    p_inf = 0
    q = 0
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
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    # Stagnation property with p = 1e5 Pa, T = 250 K, vel = 1 m/s
    p0 = 100000.68965687
    T0 = 250.00049261084
  []

  [pipe1]
    type = FlowChannel1Phase

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2

    initial_p = 1e5
    initial_T = 250
    initial_vel = 0
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1.02 0 0'
    volume = 0.1
    use_scalar_variables = false
  []

  [pipe2]
    type = FlowChannel1Phase

    position = '1.04 0 0'
    orientation = '1 0 0'
    length = 0.96
    n_elems = 2

    initial_p = 1e5
    initial_T = 250
    initial_vel = 0
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1e5
  []
[]

[Executioner]
  type = Transient
  abort_on_solve_fail = true
[]
