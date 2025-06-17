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
  [ZCyls]
    type = TestCSGCylindersMeshGenerator
    radii = "1 2"
    height = 3
    center = '10 10'
    axis = 'z'
  []
  [CylsUniverse]
    type=TestCSGUniverseMeshGenerator
    input_meshes = 'XCyls YCyls ZCyls'
    input_boxes = '10 8 15'
    bounding_box = '30 40 50'
    add_cell_to_univ_mode = true
  []
[]