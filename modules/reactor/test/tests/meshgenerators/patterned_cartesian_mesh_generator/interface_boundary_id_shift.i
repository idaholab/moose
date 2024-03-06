[Mesh]
  [square_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    background_intervals = 1
    ring_radii = 3.0
    ring_intervals = 1
    ring_block_ids = '10'
    ring_block_names = 'center_tri_1'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
    flat_side_up = true
    generate_side_specific_boundaries = true
    interface_boundary_id_shift = 100
  []
  [square_2]
      type = PolygonConcentricCircleMeshGenerator
      num_sides = 4
      num_sectors_per_side = '2 2 2 2'
      background_intervals = 1
      ring_radii = 2.5
      ring_intervals = 1
      ring_block_ids = '40'
      ring_block_names = 'center_tri_2'
      background_block_ids = 20
      background_block_names = background
      polygon_size = 5.0
      preserve_volumes = on
      flat_side_up = true
      generate_side_specific_boundaries = true
      interface_boundary_id_shift = 200
  []
  [pattern]
    type = PatternedCartesianMeshGenerator
    inputs = 'square_1 square_2'
    background_block_id=1500
    square_size=44
    duct_sizes=21
    duct_intervals=2
    duct_block_ids=2000
    pattern = '1 0 0;
               0 1 0;
               0 0 1'
    interface_boundary_id_shift_pattern = '1000 2000 3000;
                                           4000 5000 6000;
                                           7000 8000 9000'
  []
[]
