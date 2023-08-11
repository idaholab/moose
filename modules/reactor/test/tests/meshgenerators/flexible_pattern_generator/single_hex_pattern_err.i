[Mesh]
  [accg]
    type = AdvancedConcentricCircleGenerator
    num_sectors = 9
    ring_radii = '1 2'
    ring_intervals = '2 2'
    ring_block_ids = '10 15 20'
    ring_block_names = 'inner_tri inner outer'
    external_boundary_id = 100
    external_boundary_name = 'ext'
    create_outward_interface_boundaries = false
  []
  [fpg]
    type = FlexiblePatternGenerator
    inputs = 'accg'
    boundary_type = HEXAGON
    hex_patterns = '0 0;
                   0 0 0;
                    0 0'
    hex_pitches = 6
    desired_area = 1.0
  []
[]
