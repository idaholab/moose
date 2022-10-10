[GlobalParams]
    nx = 8
    ny = 8
    n_cells = 35
    pitch = 0.0136906
    rod_diameter = 0.0099568
    gap = 0.0036957
    heated_length = 1.4478
  []
  
  [Mesh]
    [subchannel]
      type = DetailedQuadSubChannelMeshGenerator
    []
    # [pins]
    #   type = DetailedQuadPinMeshGenerator
    #   input = subchannel
    # []
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
    nl_rel_tol = 0.9
    l_tol = 0.9
  []
  