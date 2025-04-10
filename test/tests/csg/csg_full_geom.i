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
  [PointSphere]
    type = TestCSGSphereAtPointMeshGenerator
    radius = 5
    x0 = -3
    y0 = -2
    z0 = -1
  []
  [OriginSphere]
    type = TestCSGSphereAtOriginMeshGenerator
    radius = 5
  []
  [CylsUniverse]
    type=TestCSGUniverseMeshGenerator
    input_meshes = 'XCyls YCyls ZCyls'
  []
  [SpheresUniverse]
    type=TestCSGUniverseMeshGenerator
    input_meshes = 'PointSphere OriginSphere'
  []
  [fullmodel]
    type=TestCSGUniverseMeshGenerator
    input_meshes = 'CylsUniverse SpheresUniverse'
  []
[]