[Mesh]
  [aclp_hex_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '10'
    polygon_size = 0.69451905
    polygon_size_style = 'apothem'
    duct_sizes_style = 'apothem'
    duct_sizes = '0.66451905 0.69201905'
    duct_intervals = '2 1'
    duct_block_ids = '1700 10'
    preserve_volumes = on
    quad_center_elements = true
    interface_boundary_id_shift = 100
  []
  [pattern_aclp]
    type = PatternedHexMeshGenerator
    inputs = 'aclp_hex_1'
    pattern_boundary = none
    generate_core_metadata = false
    pattern = '0 0;
              0 0 0;
               0 0'
    interface_boundary_id_shift_pattern = '1000 2000;
                                         3000 4000 5000;
                                           6000 7000'
  []
[]
