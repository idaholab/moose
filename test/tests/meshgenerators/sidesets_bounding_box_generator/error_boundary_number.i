[MeshGenerators]
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
    boundary_id_old = 'right'
    boundary_id_new = 11
    bottom_left = '0.5 0.5 0'
    top_right = '1.9 1.9 0'
    block_id = 0
  []

  [./createNewSidesetTwo]
    type = SideSetsFromBoundingBoxGenerator
    input = createNewSidesetOne
    boundary_id_old = 'left'
    boundary_id_new = 10
    bottom_left = '-0.1 -0.1 0'
    top_right = '0.3 0.3 0'
    block_id = 0
    boundary_id_overlap = true
  [../]
[]

[Outputs]
  exodus = true
[]
