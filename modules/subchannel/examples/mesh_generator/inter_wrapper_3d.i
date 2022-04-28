[Mesh]
  [subchannel]
    type = DetailedQuadInterWrapperMeshGenerator
    nx = 3
    ny = 3
    n_cells = 50
    assembly_pitch = 0.2
    assembly_side_x = 0.18
    assembly_side_y = 0.18
    side_bypass = 0.01
    heated_length = 3.0
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
