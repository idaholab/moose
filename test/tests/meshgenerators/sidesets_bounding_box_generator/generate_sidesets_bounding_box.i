[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    parallel_type = replicated
  []

  [./createNewSidesetOne]
    type = SideSetsFromBoundingBoxGenerator
    input = gmg
    boundaries_old = 'left'
    boundary_new = 10

    bottom_left = '-0.1 -0.1 0'
    block_id = 0
    top_right = '0.5 0.5 0'
  []
[]

[Outputs]
  exodus = true
[]
