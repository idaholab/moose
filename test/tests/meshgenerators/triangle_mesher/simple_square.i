[MeshGenerators]
  [tri]
    type = TriangleMesher
    points = '0 0 1 0 1 1 0 1'
    min_angle = 20
    max_area = 0.005
    verbose = true
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
