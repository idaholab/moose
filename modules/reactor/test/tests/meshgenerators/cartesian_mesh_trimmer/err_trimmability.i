[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
  []
  [sq_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    background_intervals = 2
    ring_radii = 4.0
    ring_intervals = 2
    ring_block_ids = '10 15'
    ring_block_names = 'center_tri center'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
  []
  [peripheral_ring]
    type = PeripheralRingMeshGenerator
    input = sq_1
    peripheral_ring_block_id = 200
    peripheral_layer_num = 2
    input_mesh_external_boundary = 10000
    peripheral_ring_radius = 8
  []
  [trim]
    type = CartesianMeshTrimmer
    input = gmg
  []
[]
