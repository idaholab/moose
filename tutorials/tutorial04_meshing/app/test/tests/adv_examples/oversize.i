[Mesh]
  [pin_regular]
    type = AdvancedConcentricCircleGenerator
    num_sectors = 12
    ring_radii = '1 2'
    ring_intervals = '2 2'
    ring_block_ids = '10 15 20'
    ring_block_names = 'inner_tri inner outer'
    external_boundary_id = 100
    external_boundary_name = 'ext'
    create_outward_interface_boundaries = false
  []
  [pin_oversize]
    type = AdvancedConcentricCircleGenerator
    num_sectors = 12
    ring_radii = '2 3'
    ring_intervals = '2 2'
    ring_block_ids = '30 35 40'
    ring_block_names = 'inner_os_tri inner_os outer_os'
    external_boundary_id = 100
    external_boundary_name = 'ext'
    create_outward_interface_boundaries = false
  []
  [fpg]
    type = FlexiblePatternGenerator
    inputs = 'pin_regular pin_oversize'
    boundary_type = HEXAGON
    boundary_sectors = 10
    boundary_size = ${fparse 22.0*sqrt(3.0)}
    hex_patterns = '0 0 0 0;
                   0 1 0 1 0;
                  0 0 0 0 0 0;
                 0 0 0 1 0 0 0;
                  0 0 0 0 0 0;
                   0 1 0 1 0;
                    0 0 0 0'
    hex_pitches = '5.5'
    hex_origins = '0.0 0.0 0.0'
    use_auto_area_func = true
  []
[]
