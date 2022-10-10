[Mesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 3.41e-2
    heated_length = 0.5334
    unheated_length_entry = 0.4064
    unheated_length_exit = 0.0762
    rod_diameter = 5.84e-3
    pitch = 7.26e-3
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
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
