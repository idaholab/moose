[Mesh]
  [pccmg]
      type = PolygonConcentricCircleMeshGenerator
      num_sides = 8
      num_sectors_per_side = '4 4 4 4 4 4 4 4'
      background_intervals = 2
      polygon_size = 5.0

      preserve_volumes = on

      tri_element_type = TRI6
      quad_element_type = QUAD8
  []
  [pr]
      type = PeripheralRingMeshGenerator
      input = pccmg
      peripheral_layer_num = 4
      peripheral_ring_radius = 15
      input_mesh_external_boundary = 10000
      peripheral_ring_block_id = 250
      peripheral_ring_block_name = reactor_ring

      peripheral_inner_boundary_layer_bias = 2.0
      peripheral_inner_boundary_layer_intervals = 3
      peripheral_inner_boundary_layer_width = 1

      peripheral_outer_boundary_layer_bias = 0.5
      peripheral_outer_boundary_layer_intervals = 3
      peripheral_outer_boundary_layer_width = 1

      peripheral_radial_bias = 1.5
  []
[]
