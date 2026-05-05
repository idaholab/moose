expected = '56 8'

!include check.i

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []

  [stl]
    type = FileMeshGenerator
    file = cube_ascii.stl
  []
  [scale]
    type = TransformGenerator
    input = stl
    transform = SCALE
    vector_value = '1 1 1'
  []
  [rotate]
    type = TransformGenerator
    input = scale
    transform = ROTATE
    vector_value = '0 0 0'
  []
  [translate]
    type = TransformGenerator
    input = rotate
    transform = TRANSLATE
    vector_value = '0.5 0.5 0.5'
  []
  [apply]
    type = ManifoldSubdomainGenerator
    input = gmg
    manifold = translate
    block_id = 1
  []
[]
