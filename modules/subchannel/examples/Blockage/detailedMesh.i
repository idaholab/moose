[GlobalParams]
    nx = 8
    ny = 8
    n_cells = 36
    pitch = 0.0136906
    pin_diameter = 0.0099568
    gap = 0.0036957
    heated_length = 1.4224
  []

  [Mesh]
    [subchannel]
      type = DetailedQuadSubChannelMeshGenerator
    []
  []

  [AuxVariables]
    [mdot]
    []
    [SumWij]
    []
    [P]
    []
    [DP]
    []
    [h]
    []
    [T]
    []
    [Tpin]
    []
    [Dpin]
    []
    [rho]
    []
    [mu]
    []
    [S]
    []
    [w_perim]
    []
    [q_prime]
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

