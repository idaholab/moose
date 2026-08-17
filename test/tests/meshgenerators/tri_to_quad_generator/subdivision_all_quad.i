[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [tri]
    type = ElementsToSimplicesConverter
    input = gmg
  []
  [to_quad]
    type = TriToQuadGenerator
    input = tri
    algorithm = SUBDIVISION
  []

  # The subdivision of each triangle depends on element id numbering
  allow_renumbering = false
[]
