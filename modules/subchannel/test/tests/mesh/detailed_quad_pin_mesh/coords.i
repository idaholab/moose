[GlobalParams]
  nx = 3
  ny = 3
  n_cells = 2
  pitch = 1
  gap = 0.1
  heated_length = 1
  rod_diameter = 0.5
[]

[Mesh]
  [subchannel]
    type = DetailedQuadSubChannelMeshGenerator
  []
  [pins]
    type = DetailedQuadPinMeshGenerator
    input = subchannel
  []
[]
