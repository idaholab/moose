[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 2
    ymax = 2
    nx = 2
    ny = 2
  []

  [refine]
    type = RefineBlockGenerator
    input = gmg
    block = 0
    refinement = 1
  []

  [lower_left]
    type = ParsedGenerateSideset
    input = refine
    combinatorial_geometry = 'y < 1.1'
    new_sideset_name = lower_left
    included_boundaries = left
  []
[]

[Outputs]
  exodus = true
[]
