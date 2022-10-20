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
  []
[]
