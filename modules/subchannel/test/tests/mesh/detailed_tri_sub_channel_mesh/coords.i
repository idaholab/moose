[GlobalParams]
  nrings = 3
  n_cells = 20
  flat_to_flat = 0.056
  heated_length = 0.2
  pitch = 0.012
[]

[TriSubChannelMesh]
  [subchannel]
    type = SCMDetailedTriSubChannelMeshGenerator
    pin_diameter = 0.01
    verbose_flag =  true
  []
[]
