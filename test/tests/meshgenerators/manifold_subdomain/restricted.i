expected = '32 4 28'

!include check.i

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
  [seed]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '0.5 1 1'
    block_id = 2
  []

  [stl]
    type = FileMeshGenerator
    file = cube_ascii.stl
  []
  [translate]
    type = TransformGenerator
    input = stl
    transform = TRANSLATE
    vector_value = '0.5 0.5 0.5'
  []
  [apply]
    type = ManifoldSubdomainGenerator
    input = seed
    manifold = translate
    restricted_subdomains = '2'
    block_id = 1
  []
[]
