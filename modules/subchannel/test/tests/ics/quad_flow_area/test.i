[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    pin_diameter = 0.125
    gap = 0.1
    heated_length = 1
    spacer_k = '0.0'
    spacer_z = '0'
  []
[]

[Variables]
  [S]
  []
[]

[ICs]
  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
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
