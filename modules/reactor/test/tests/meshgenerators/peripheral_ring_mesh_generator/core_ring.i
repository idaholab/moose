[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = input_mesh.e
  []
  [tg]
    input = fmg
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '0 1 0'
  []
  [pr]
    type = PeripheralRingMeshGenerator
    input = fmg
    peripheral_layer_num = 3
    peripheral_ring_radius = 80.0
    input_mesh_external_boundary = 10000
    peripheral_ring_block_id = 250
    peripheral_ring_block_name = reactor_ring
  []
[]
