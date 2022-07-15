[Mesh]
  [subchannel]
    type = DetailedQuadInterWrapperMeshGenerator
    nx = 5
    ny = 5
    n_cells = 3
    assembly_pitch = 0.2
    assembly_side_x = 0.18
    assembly_side_y = 0.18
    side_bypass = 0.001
    heated_length = 1.0
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
