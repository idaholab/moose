[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[NodalNormals]
  [./nodal_normals_all]
    boundary = 'left right top bottom'
    execute_on = initial
  [../]
[]

[AuxVariables]
  [./nodal_normal_x]
  [../]
  [./nodal_normal_y]
  [../]
  [./nodal_normal_z]
  [../]
[]

[AuxKernels]
  [./nnx]
    type = NodalNormalAux
    variable = nodal_normal_x
    component = X
    nodal_normals = nodal_normals_all
    execute_on = timestep_end
  [../]
  [./nny]
    type = NodalNormalAux
    variable = nodal_normal_y
    component = Y
    nodal_normals = nodal_normals_all
    execute_on = timestep_end
  [../]
  [./nnz]
    type = ConstantAux
    variable = nodal_normal_z
    value = 0
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
