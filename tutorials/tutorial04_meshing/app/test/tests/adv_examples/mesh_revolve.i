[Mesh]
  [shg]
    type = SimpleHexagonGenerator
    hexagon_size = 1.0
    block_id = '100 105'
    block_name = 'hex_tri hex'
    element_type = HYBRID
    radial_intervals = 2
  []
  [rg]
    type = RevolveGenerator
    input = shg
    axis_point = '5.0 0.0 0.0'
    axis_direction = '0.0 1.0 0.0'
    nums_azimuthal_intervals = '4 8 16 32'
    revolving_angles = '20 40 60 80'
    subdomain_swaps = ' ;
                       100 200 105 205;
                       100 300 105 305;
                       100 110'
  []
[]
