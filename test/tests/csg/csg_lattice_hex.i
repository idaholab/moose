[Mesh]
  [sq1]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 4
  []
  [sq2]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 3
  []
  [hex_lat]
    type = TestCSGLatticeMeshGenerator
    lattice_type = 'hexagonal'
    inputs = 'sq1 sq2'
    pattern =  '0 0 0;
               0 1 1 0;
              0 1 0 1 0;
               0 1 1 0;
                0 0 0'
    pitch = 5
  []
  [sq3]
      type = ExampleCSGInfiniteSquareMeshGenerator
      side_length = 30
      fill = 'hex_lat'
  []
[]
