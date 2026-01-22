[Mesh]
  [sq1]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 2
  []
  [lat1]
    type = TestCSGLatticeMeshGenerator
    lattice_type = 'cartesian'
    inputs = 'sq1'
    pattern = '0 0;
               0 0'
    pitch = 3
  []
  [sq2]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 7
    fill = 'lat1'
  []
  [lat2]
    type = TestCSGLatticeMeshGenerator
    lattice_type = 'cartesian'
    inputs = 'sq2'
    pattern = '0 0 0;
               0 0 0;
               0 0 0'
    pitch = 10
  []
  [sq3]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 35
    fill = 'lat2'
  []
[]
