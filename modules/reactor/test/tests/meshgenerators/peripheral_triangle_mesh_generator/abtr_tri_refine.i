[Mesh]
  allow_renumbering = 0
  [hex_in]
    type = FileMeshGenerator
    file = gold/abtr_mesh.e
  []

  [tmg]
    type = PeripheralTriangleMeshGenerator
    input = hex_in
    peripheral_ring_radius = 150
    peripheral_ring_num_segments = 50
    desired_area = 50
    peripheral_ring_block_name = 'periphery'
  []
[]

[Postprocessors]
  [periphery_area]
    type = VolumePostprocessor
    block = periphery
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  file_base = abtr_tri_refine
[]
