[Mesh]
  [shg]
    type = SimpleHexagonGenerator
    hexagon_size = 1.0
    block_id = 100
    element_type = QUAD
  []
  [fpg]
    type = FlexiblePatternGenerator
    inputs = 'shg'
    boundary_type = HEXAGON
    boundary_size = 3
    boundary_sectors = 1
    extra_positions = '0 0 0'
    extra_positions_mg_indices = '0'
    background_subdomain_id = 200
  []
[]
