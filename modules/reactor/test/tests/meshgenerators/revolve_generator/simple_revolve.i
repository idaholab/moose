[Mesh]
  [shg]
    type = SimpleHexagonGenerator
    hexagon_size = 1.0
    block_id = 100
    block_name = hexagon
    external_boundary_id  = 300
    external_boundary_name = 'external_side'
  []
  [rg]
    type = RevolveGenerator
    input = shg
    axis_point = '5.0 0.0 0.0'
    axis_direction = '0.0 1.0 0.0'
    nums_azimuthal_intervals = 6
  []
[]
