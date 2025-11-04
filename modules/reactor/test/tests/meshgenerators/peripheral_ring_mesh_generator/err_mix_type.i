[Mesh]
  [obdry_1]
    type = PolyLineMeshGenerator
    points = '-1 0 0
              0 0 0
              0 1 0
              -1 1 0'
    loop = true
    num_edges_between_points = 2
  []
  [tri_1]
    type = XYDelaunayGenerator
    boundary = 'obdry_1'
    refine_boundary = false
    output_subdomain_name = 1
    output_boundary = 10
  []
  [obdry_2]
    type = PolyLineMeshGenerator
    points = '1 0 0
              0 0 0
              0 1 0
              1 1 0'
    loop = true
    num_edges_between_points = 1
  []
  [tri_2]
    type = XYDelaunayGenerator
    boundary = 'obdry_2'
    refine_boundary = false
    output_subdomain_name = 2
    output_boundary = 10
  []
  [convert]
    type = ElementOrderConversionGenerator
    input = tri_2
    conversion_type = SECOND_ORDER
  []
  [smg]
    type = StitchedMeshGenerator
    inputs = 'tri_1 convert'
    stitch_boundaries_pairs = '10 10'
    parallel_type = 'replicated'
    prevent_boundary_ids_overlap = false
  []
  [pr]
    type = PeripheralRingMeshGenerator
    input = smg
    peripheral_layer_num = 4
    peripheral_ring_radius = 15
    input_mesh_external_boundary = 10
    peripheral_ring_block_id = 250
    peripheral_ring_block_name = reactor_ring
  []
[]
