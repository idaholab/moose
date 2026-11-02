  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 50

    A   = 1.0000000000e-04
    D_h  = 1.1283791671e-02

    f = 0.0

    fp = eos
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:in'
    p0 = 1e6
    T0 = 453.1
    reversible = false
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 0.5e6
  []
