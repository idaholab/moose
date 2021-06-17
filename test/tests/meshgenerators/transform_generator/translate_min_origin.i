[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = 1
    xmax = 2
    ymin = 3
    ymax = 4
  []

  [translate]
    type = TransformGenerator
    input = gmg
    transform = translate_min_origin
  []
[]
