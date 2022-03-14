[GlobalParams]
  heated_length = 1.0
  nrings = 5
  flat_to_flat = 1.2
  pitch = 0.13
  n_cells = 2
[]

[TriSubChannelMesh]
  [sub_channel]
    type = TriSubChannelMeshGenerator
    rod_diameter = 0.1
    dwire = 0.03
    hwire = 0.3
    spacer_k = '0.5'
    spacer_z = '0'
  []

  [duct]
    type = TriDuctMeshGenerator
    input = sub_channel
  []
[]
