[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
    elem_type = TET4
  []

  [./left_block]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    block_name = left_block
    bottom_left = '0 0 0'
    top_right = '0.5 1 1'
  []
  
  [./right_block]
    type = SubdomainBoundingBoxGenerator
    input = left_block
    block_id = 2
    block_name = right_block
    bottom_left = '0.5 0 0'
    top_right = '1 1 1'
  []

  [./center_side_set]
    type = SideSetsBetweenSubdomainsGenerator
    input = right_block
    master_block = left_block
    paired_block = right_block
    new_boundary = center_side_set
  []

  [./center_mesh]
    type = MeshSideSetGenerator
    input = center_side_set
    boundaries = center_side_set
    block_id = 10
    block_name = center_mesh
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
