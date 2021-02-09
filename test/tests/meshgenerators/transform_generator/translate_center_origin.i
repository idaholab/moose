[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []

  [translate]
    type = TransformGenerator
    input = gmg
    transform = translate_center_origin
  []
[]
