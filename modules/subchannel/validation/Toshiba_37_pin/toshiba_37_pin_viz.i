[Mesh]
  [subchannel]
    type = SCMDetailedTriAssemblyMeshGenerator
    nrings = 4
    n_cells = 20
    flat_to_flat = 0.085
    heated_length = 1.0
    pin_diameter = 0.01
    pitch = 0.012
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
