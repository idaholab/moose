################################################################################
# Inputs relevant to the subchannel solver
################################################################################

[Mesh]
  type = DetailedSubChannelMesh
  # nx = 6 #subchannel number in the x direction
  # ny = 6
  # max_dz = 0.02
  # pitch = 0.0126
  # rod_diameter = 0.00950
  # gap = 0.00095
  # heated_length = 3.658
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [SumWijh]
  []
  [SumWijPrimeDhij]
  []
  [SumWijPrimeDUij]
  []
  [P]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [q_prime]
  []
[]

[Outputs]
  exodus = true
[]

################################################################################
# Stuff needed to make the program execute
################################################################################

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
