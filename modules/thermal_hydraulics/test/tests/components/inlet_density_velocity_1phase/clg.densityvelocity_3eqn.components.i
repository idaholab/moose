  [pipe]
    type = FlowChannel1Phase
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10

    A = 1.907720E-04
    f = 0.0

    fp = eos
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'

    rho = 996.556340388366266
    vel = 2
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 0.1e6
  []
