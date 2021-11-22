[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    n_blocks = 1
    pitch = 0.25
    rod_diameter = 0.125
    gap = 0.1
    heated_length = 1
    spacer_k = '0.0'
    spacer_z = '0'
  []
[]

[Variables]
  [w_perim]
  []
[]

[ICs]
  [w_perim_IC]
    type = QuadWettedPerimIC
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
