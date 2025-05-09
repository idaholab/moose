[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [./createNewSidesetOne]
    type = SideSetsFromBoundingBoxGenerator
    input = gmg
    included_boundaries = 'left'
    boundary_new = 10

    bottom_left = '-0.1 -0.1 0'
    top_right = '0.5 0.5 0'
  []
[]

[Outputs]
  exodus = true
[]
