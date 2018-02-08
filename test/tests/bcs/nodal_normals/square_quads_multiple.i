# This tests nodal normals on multiple sides sharing a common node where we
# would have 2 different normals.  Each normal is then picked up by the
# object that is restricted to a different boundary ID.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[MeshModifiers]
  [./left_corners_mm]
    type = AddExtraNodeset
    new_boundary = 'left_corners'
    coord = '0 0 0 1'
  [../]

  [./bottom_corners_mm]
    type = AddExtraNodeset
    new_boundary = 'bottom_corners'
    coord = '0 0 1 0'
  [../]
[]

[NodalNormals]
  [./nodal_normals_left]
    boundary = 'left'
    corner_boundary = left_corners
    execute_on = initial
  [../]

  [./nodal_normals_bottom]
    boundary = 'bottom'
    corner_boundary = bottom_corners
    execute_on = initial
  [../]
[]

[AuxVariables]
  [./nodal_normal_left_x]
  [../]
  [./nodal_normal_left_y]
  [../]
  [./nodal_normal_left_z]
  [../]

  [./nodal_normal_bottom_x]
  [../]
  [./nodal_normal_bottom_y]
  [../]
  [./nodal_normal_bottom_z]
  [../]
[]

[AuxKernels]
  [./nnx_left]
    type = NodalNormalAux
    variable = nodal_normal_left_x
    component = X
    nodal_normals = nodal_normals_left
    execute_on = timestep_end
  [../]
  [./nny_left]
    type = NodalNormalAux
    variable = nodal_normal_left_y
    component = Y
    nodal_normals = nodal_normals_left
    execute_on = timestep_end
  [../]
  [./nnz_left]
    type = NodalNormalAux
    variable = nodal_normal_left_z
    component = Z
    nodal_normals = nodal_normals_left
    execute_on = timestep_end
  [../]

  [./nnx_bottom]
    type = NodalNormalAux
    variable = nodal_normal_bottom_x
    component = X
    nodal_normals = nodal_normals_bottom
    execute_on = timestep_end
  [../]
  [./nny_bottom]
    type = NodalNormalAux
    variable = nodal_normal_bottom_y
    component = Y
    nodal_normals = nodal_normals_bottom
    execute_on = timestep_end
  [../]
  [./nnz_bottom]
    type = NodalNormalAux
    variable = nodal_normal_bottom_z
    component = Z
    nodal_normals = nodal_normals_bottom
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
