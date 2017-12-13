# This makes sure that nodal normals are correctly computed on an internal sideset

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[MeshModifiers]
  [./left_block]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  [../]
  [./right_block]
    type = SubdomainBoundingBox
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  [../]

  [./internal_wrt_left]
    type = SideSetsBetweenSubdomains
    depends_on = 'left_block right_block'
    master_block = 1
    paired_block = 2
    new_boundary = int_wrt_left
  [../]

  [./internal_wrt_right]
    type = SideSetsBetweenSubdomains
    depends_on = 'left_block right_block'
    master_block = 2
    paired_block = 1
    new_boundary = int_wrt_right
  [../]

  [./internal_corner]
    type = AddExtraNodeset
    new_boundary = corner
    coord = '0.5 0 0.5 1'
  [../]
[]

[NodalNormals]
  [./nodal_normals_left_blk]
    block = 1
    boundary = 'int_wrt_left'
    execute_on = initial
  [../]

  [./nodal_normals_right_blk]
    block = 2
    boundary = 'int_wrt_right'
    corner_boundary = corner
    execute_on = initial
  [../]
[]

[AuxVariables]
  [./nodal_normal_left_x]
  [../]
  [./nodal_normal_left_y]
  [../]

  [./nodal_normal_right_x]
  [../]
  [./nodal_normal_right_y]
  [../]
[]

[AuxKernels]
  [./nnx_left]
    type = NodalNormalAux
    variable = nodal_normal_left_x
    component = X
    nodal_normals = nodal_normals_left_blk
    execute_on = timestep_end
  [../]
  [./nny_left]
    type = NodalNormalAux
    variable = nodal_normal_left_y
    component = Y
    nodal_normals = nodal_normals_left_blk
    execute_on = timestep_end
  [../]

  [./nnx_right]
    type = NodalNormalAux
    variable = nodal_normal_right_x
    component = X
    nodal_normals = nodal_normals_right_blk
    execute_on = timestep_end
  [../]
  [./nny_right]
    type = NodalNormalAux
    variable = nodal_normal_right_y
    component = Y
    nodal_normals = nodal_normals_right_blk
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
