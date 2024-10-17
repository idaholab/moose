[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 3
    n_cells = 2
    flat_to_flat = 0.60
    heated_length = 1.0
    pin_diameter = 0.1
    pitch = 0.13
    dwire = 0.03
    hwire = 0.3
    spacer_k = '0.5'
    spacer_z = '0'
  []
[]

[Variables]
  [w_perim]
  []
[]

[ICs]
  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
