[Mesh]
  [csg_inf_square]
    type = TestCSGInfiniteSquareMeshGenerator
    side_length = 5
  []
  [csg_cube]
    type = TestCSGAxialSurfaceMeshGenerator
    input = csg_inf_square
    axial_height = 5
  []
[]
