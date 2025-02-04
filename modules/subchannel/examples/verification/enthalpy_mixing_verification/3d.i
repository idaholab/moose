[Mesh]
    [subchannel]
      type = SCMDetailedQuadSubChannelMeshGenerator
      nx = 2
      ny = 1
      n_cells = 100
      pitch = 0.0126
      pin_diameter = 0.00950
      gap = 0.00095 #
      heated_length = 10.0
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
