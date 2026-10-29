  [pipe]
    type = FlowChannel1Phase
    fp = fp
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A   = 1.0000000000e-04
    D_h  = 1.1283791671e-02
    f = 0.1
    length = 1
    n_elems = 4
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    m_dot = 0.18
    T     = 444.447
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 7e6
  []
