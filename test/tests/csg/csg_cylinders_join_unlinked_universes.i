[Mesh]
  [XCyls]
    type = TestCSGCylindersMeshGenerator
    radii = "1 2 3"
    height = 4
    center = '0 0'
    axis = 'x'
  []
  [YCyls]
    type = TestCSGCylindersMeshGenerator
    radii = "2 4"
    height = 2
    center = '-10 -10'
    axis = 'y'
  []
  [CylsUniverse]
    type=TestCSGUnlinkedUniverseMeshGenerator
    input_meshes = 'XCyls YCyls'
  []
[]
