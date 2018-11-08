[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 1
    ymax = 1
    uniform_refine = 2
  []

  [./subdomains]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0.1 0.1 0'
    block_id = 1
    top_right = '0.9 0.9 0'
    location = OUTSIDE
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
