[Mesh]
  [gen]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 5
    num_sectors_per_side = '4 4 4 4 4'
    background_intervals = 2
    polygon_size = 5.0

    preserve_volumes = on
    external_boundary_id = 9999
    external_boundary_name = 'polygon_out'
    generate_side_specific_boundaries = true
  []
[]
