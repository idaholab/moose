  [pipe]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A   = 1.0000000000e-04
    f = 0.0
    length = 1
    n_elems = 100
  []

  [inlet]
    type = InletVelocityTemperature1Phase
    input = 'pipe:in'
    vel = 1.0
    T     = 444.447
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 7e6
  []
