[Mesh]
  [PointSphere]
    type = TestCSGSphereAtPointMeshGenerator
    radius = 5
    center = '-3 -2 -1'
  []
  [OriginSphere]
    type = TestCSGSphereAtOriginMeshGenerator
    radius = 3
  []
  [inf_square]
    type = ExampleCSGInfiniteSquareMeshGenerator
    side_length = 5
  []
  [Cube]
    type = TestCSGAxialSurfaceMeshGenerator
    input = inf_square
    axial_height = 5
  []
  [ComboUniverse]
    type=TestCSGJoinBasesMeshGenerator
    input_meshes = 'PointSphere OriginSphere Cube'
  []
[]
