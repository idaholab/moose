[Mesh]
  [hex_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 4
    ring_radii = '2.0 4.0'
    ring_intervals = '5 5'
    ring_block_ids = '10 11 15'
    ring_block_names = 'center_tri center mid'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
    ring_radial_biases = '1.0 1.6'
    background_radial_bias = 0.625
  []
[]
