[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
  []
  [SubdomainBoundingBox]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '2 2 2'
  []
  [ed0]
    type = BlockDeletionGenerator
    input = SubdomainBoundingBox
    block = 1

    # this makes a new boundary with an ID of 100 and a name of "100"
    new_boundary = '100'
  []
  [rename_both_id_and_name]
    type = RenameBoundaryGenerator
    input = ed0
    old_boundary = '100' # this is both an ID and a name, which we want to both rename
    new_boundary = '101'
  []

  # We compare by element numbers, which are not consistent in parallel
  # if this is true
  allow_renumbering = false

  parallel_type = replicated
[]

[Reporters/mesh_info]
  type = MeshInfo
  items = sideset_elems
[]

[Outputs]
  [out]
    type = JSON
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
