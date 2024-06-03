[Mesh]
  [accg]
    type = AdvancedConcentricCircleGenerator
    ring_radii = '2'
    ring_intervals = '1'
    ring_block_ids = '10'
    ring_block_names = 'inner'
    num_sectors = 12
    tri_element_type = TRI6
  []
  [fpg]
    type = FlexiblePatternGenerator
    inputs = 'accg'
    boundary_type = HEXAGON
    boundary_size = 5
    boundary_sectors = 5
    extra_positions = '0.0 0.0 0.0'
    extra_positions_mg_indices = '0'
    desired_area = 0.8
    tri_element_type = TRI6
  []
[]
