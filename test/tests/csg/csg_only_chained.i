[Mesh]
  [inf_square]
    type = TestCSGInfiniteSquareMeshGenerator
    side_length = 5
  []
  [cube]
    type = TestCSGAxialSurfaceMeshGenerator
    input = inf_square
    axial_height = 5
  []
[]
