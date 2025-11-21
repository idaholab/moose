[Mesh]
  [sq1]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 4
  []
  [sq2]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 3
  []
  [cart_lat]
    type = TestCSGLatticeMeshGenerator
    lattice_type = 'cartesian'
    inputs = 'sq1 sq2'
    pattern = '0 0;
               0 1'
    pitch = 5
  []
  [sq3]
      type = ExampleCSGInfiniteSquareMeshGenerator
      side_length = 15
      fill = 'cart_lat'
  []
[]
