[Mesh]
  [PointSphere]
    type = TestCSGSphereAtPointMeshGenerator
    radius = 5
    center = '-3 -2 -1'
  []
  [OriginSphere]
    type = TestCSGSphereAtOriginMeshGenerator
    radius = 5
  []
  [SpheresUniverse]
    type=TestCSGUniverseMeshGenerator
    input_meshes = 'PointSphere OriginSphere'
    bounding_box = '20 20 20'
  []
[]