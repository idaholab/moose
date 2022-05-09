[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    rod_diameter = 0.125
    gap = 0.1
    heated_length = 1
    spacer_k = '0.0'
    spacer_z = '0'
  []

  [fuel_pins]
    type = QuadPinMeshGenerator
    input = sub_channel
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.25
    heated_length = 1
  []
[]
