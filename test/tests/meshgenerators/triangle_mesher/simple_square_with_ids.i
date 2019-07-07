[MeshGenerators]
  [tri]
    type = TriangleMesher
    points = '0 0 1 0 1 1 0 1'
    min_angle = 20
    max_area = 0.005
    verbose = true

    segment_markers = '2 3 4 5'
    regions = '0.5 0.5'
    region_subdomain_ids = 2
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
