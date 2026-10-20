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

  [in]
    type = InletVelocityTemperature1Phase
    input = 'pipe:in'
    vel = -1.0
    T     = 444.447
  []

  [out]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:out'
    p0 = 7e6
    T0 = 444.447
  []
