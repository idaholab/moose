[Mesh]
  [subchannel]
    type = SCMDetailedTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 0.05319936
    heated_length = 0.87
    unheated_length_entry = 0.0
    unheated_length_exit = 0.402
    pin_diameter = 8.2e-3
    pitch = 0.01148
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
