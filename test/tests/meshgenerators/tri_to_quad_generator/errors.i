[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    elem_type = TRI3
  []
  [to_quad]
    type = TriToQuadGenerator
    input = gmg
  []
[]
