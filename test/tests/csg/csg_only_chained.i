[Mesh]
  [inf_square]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 4
  []
  [cube]
    type = TestCSGAxialSurfaceMeshGenerator
    input = inf_square
    axial_height = 5
  []
[]
