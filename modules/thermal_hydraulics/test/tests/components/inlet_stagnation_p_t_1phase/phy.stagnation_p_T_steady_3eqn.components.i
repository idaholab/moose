  [pipe]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1.
    f = 0.0

    length = 1
    n_elems = 10
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 102041.128
    T0 = 300.615
    reversible = false
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 101325
  []
