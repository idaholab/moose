  [pipe]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A   = 3.1415926536e-06
    D_h  = 2.0000000000e-03
    f = 0.1
    length = 1
    n_elems = 10
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'
    rho = 805
    vel = 1
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 7e6
  []
