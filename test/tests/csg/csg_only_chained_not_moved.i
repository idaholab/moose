[Mesh]
  [csg_inf_square]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 5
  []
  [csg_cube]
    type = TestCSGInputNotMovedMeshGenerator
    input = csg_inf_square
  []
[]
