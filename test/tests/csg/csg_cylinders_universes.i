[Mesh]
  [XCyls]
    type = TestCSGCylindersMeshGenerator
    radii = "1 2 3"
    height = 4
    x0 = 0
    x1 = 0
    axis = 'x'
  []
  [YCyls]
    type = TestCSGCylindersMeshGenerator
    radii = "2 4"
    height = 2
    x0 = 4
    x1 = 3
    axis = 'y'
  []
  [ZCyls]
    type = TestCSGCylindersMeshGenerator
    radii = "1 2"
    height = 2
    x0 = 10
    x1 = 10
    axis = 'z'
  []
  [CylsUniverse]
    type=TestCSGUniverseMeshGenerator
    input_meshes = 'XCyls YCyls ZCyls'
    input_boxes = '10 20 15'
    bounding_box = '30 40 50'
  []
[]