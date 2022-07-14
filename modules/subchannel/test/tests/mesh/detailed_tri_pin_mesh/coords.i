[GlobalParams]
  nrings = 3
  n_cells = 20
  flat_to_flat = 0.056
  heated_length = 0.2
  pitch = 0.012
  rod_diameter = 0.01
[]

[TriSubChannelMesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    dwire = 0.002
    hwire = 0.0833
    spacer_z = '0'
    spacer_k = '5.0'
  []
  [pins]
    type = DetailedTriPinMeshGenerator
    input = subchannel
  []
[]
