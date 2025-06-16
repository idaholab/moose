[Mesh]
  [squareA]
    type = TestCSGInfiniteSquareMeshGenerator
    side_length = 5
  []
  [squareB]
    type = TestCSGInfiniteSquareMeshGenerator
    side_length = 10
  []
  [combo]
    type=TestCSGUniverseCellError
    input_meshes = 'squareA squareB'
    mode = 'add'
  []
[]