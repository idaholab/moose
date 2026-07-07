[GlobalParams]
    nx = 8
    ny = 8
    n_cells = 36
    pitch = 0.0136906
    pin_diameter = 0.0099568
    side_gap = 0.0036957
    heated_length = 1.4224
  []

  [Mesh]
    [assembly]
      type = SCMDetailedQuadAssemblyMeshGenerator
    []
  []

  [AuxVariables]
    [mdot]
      block = subchannel
    []
    [SumWij]
      block = subchannel
    []
    [P]
      block = subchannel
    []
    [DP]
      block = subchannel
    []
    [h]
      block = subchannel
    []
    [T]
      block = subchannel
    []
    [Tpin]
      block = fuel_pins
    []
    [Dpin]
      block = fuel_pins
    []
    [rho]
      block = subchannel
    []
    [mu]
      block = subchannel
    []
    [S]
      block = subchannel
    []
    [w_perim]
      block = subchannel
    []
    [q_prime]
      block = fuel_pins
    []
  []

  [Problem]
    type = NoSolveProblem
  []

  [Outputs]
    exodus = true
  []

  [Executioner]
    type = Steady
  []
