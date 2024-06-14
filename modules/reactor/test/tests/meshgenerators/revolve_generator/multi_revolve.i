[Mesh]
  [shg]
    type = SimpleHexagonGenerator
    hexagon_size = 1.0
    element_type = HYBRID
    block_id = '100 200'
    block_name = 'block_tri block_quad'
    external_boundary_id = 300
    external_boundary_name = 'external_side'
    radial_intervals = 2
  []
  [rg]
    type = RevolveGenerator
    input = shg
    axis_point = '5.0 0.0 0.0'
    axis_direction = '0.0 1.0 0.0'
    nums_azimuthal_intervals = '6 6'
    revolving_angles = '150 210'
    subdomain_swaps = ' ;100 300 200 400'
    boundary_swaps = '300 500;10000 4000'
  []
[]
