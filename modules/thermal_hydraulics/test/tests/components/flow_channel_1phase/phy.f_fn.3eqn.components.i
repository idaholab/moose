  [pipe]
    type = FlowChannel1Phase
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1

    A   = 1.907720E-04
    D_h  = 1.698566E-02
    f = f_func

    fp = eos
  []

  [ht_pipe]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 559
    P_hf = 0.0489623493599167
    Hw = 50000
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'
    rho = 741.707129779398883
    vel = 2
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 7.0e6
  []
