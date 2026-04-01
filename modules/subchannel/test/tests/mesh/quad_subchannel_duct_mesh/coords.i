[GlobalParams]
  nx = 5
  ny = 4
  n_cells = 10
  pitch = 0.25
  pin_diameter = 0.125
  side_gap = 0.1
  heated_length = 1
  spacer_k = '0.0'
  spacer_z = '0'
[]

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
  []

  [duct]
    type = SCMQuadDuctMeshGenerator
    input = sub_channel
  []
[]

