[Mesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = 1 #3
    n_cells = 40
    flat_to_flat = 1.2e-2 # 3.41e-2
    heated_length = 1.0 # 0.5334
    # unheated_length_entry = 0.4064
    # unheated_length_exit = 0.0762
    rod_diameter = 5.84e-3
    pitch = 7.26e-3
  []

  [fuel_pins]
    type = DetailedTriPinMeshGenerator
    input = subchannel
    nrings = 1 #3
    n_cells = 40
    heated_length = 1.0 # 0.5334
    # unheated_length_entry = 0.4064
    # unheated_length_exit = 0.0762
    rod_diameter = 5.84e-3
    pitch = 7.26e-3
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
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
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
