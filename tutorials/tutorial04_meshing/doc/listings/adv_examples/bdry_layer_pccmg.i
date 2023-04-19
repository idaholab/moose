[Mesh]
  [hex_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 2
    ring_radii = '2.0 4.0'
    ring_intervals = '2 2'
    ring_block_ids = '10 11 15'
    ring_block_names = 'center_tri center mid'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
    ## Background boundary layer setting
    background_inner_boundary_layer_bias = 1.6
    background_inner_boundary_layer_intervals = 3
    background_inner_boundary_layer_width = 0.4
    ## Ring boundary layer setting
    ring_outer_boundary_layer_biases = '1.0 0.625'
    ring_outer_boundary_layer_intervals = '0 3'
    ring_outer_boundary_layer_widths = '0.0 0.5'
  []
[]
