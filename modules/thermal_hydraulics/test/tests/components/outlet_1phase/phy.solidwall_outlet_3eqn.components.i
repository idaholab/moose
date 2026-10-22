  [pipe]
    type = FlowChannel1Phase
    fp = fp

    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 100
    A = 1.0

    # IC
    initial_T = 400
    initial_p = 1e5
    initial_vel = 0

    f = 0
  []

  [left_wall]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 0.95e5
  []
