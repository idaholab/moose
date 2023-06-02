[Mesh]
  [hex_1]
    type = SimpleHexagonGenerator
    hexagon_size = 1.0
    block_id = 1000
    block_name = hex_1000
  []
  [pattern]
    type = PatternedHexMeshGenerator
    inputs = 'hex_1'
    pattern = '0 0;
              0 0 0;
               0 0'
    pattern_boundary = none
  []
[]
