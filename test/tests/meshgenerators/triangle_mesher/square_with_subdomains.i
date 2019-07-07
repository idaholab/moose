[MeshGenerators]
  [tri]
    type = TriangleMesher
    points = '0 0 1 0 1 1 0 1 0.5 0 0.5 1'
    min_angle = 20
    max_area = 0.005
    verbose = true

    # require the explict segments to cut the geometry
    segments = '0 1 1 2 2 3 3 0 4 5'
    regions = '0.25 0.5 0.75 0.5'
    region_subdomain_ids = '0 1'
    region_max_areas = '0 0.001'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
