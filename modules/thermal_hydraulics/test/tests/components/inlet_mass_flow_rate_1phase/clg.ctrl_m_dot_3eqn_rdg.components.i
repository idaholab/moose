  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 50

    A   = 1.0000000000e-04
    D_h = 1.1283791671e-02

    f = 0.0

    fp = fp
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe:in'
    m_dot = 0
    T = 300
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1e5
  []
