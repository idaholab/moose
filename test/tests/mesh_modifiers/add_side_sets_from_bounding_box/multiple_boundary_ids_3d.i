[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  # This MeshModifier currently only works with ReplicatedMesh.
  # For more information, refer to #2129.
  parallel_type = replicated
[]

[MeshModifiers]
  [./createNewSidesetOne]
    type = AddSideSetsFromBoundingBox
    boundary_id_old = 'left bottom front'
    boundary_id_new = 10
    bottom_left = '-0.1 -0.1 -0.1'
    top_right = '0.1 0.2 0.3'
    block_id = 0
  [../]
  [./createNewSidesetTwo]
    type = AddSideSetsFromBoundingBox
    boundary_id_old = 'right top back'
    boundary_id_new = 11
    bottom_left = '0.6 0.7 0.8'
    top_right = '1.1 1.1 1.1'
    block_id = 0
  [../]
  [./createNewSidesetThree]
    type = AddSideSetsFromBoundingBox
    boundary_id_old = 'left top back'
    boundary_id_new = 12
    bottom_left = '-0.1 0.9 0.9'
    top_right = '0.1 1.1 1.1'
    block_id = 0
  [../]
  [./createNewSidesetFour]
    type = AddSideSetsFromBoundingBox
    boundary_id_old = 'front'
    boundary_id_new = 13
    bottom_left = '0.4 0.4 0.9'
    top_right = '0.6 0.6 1.1'
    block_id = 0
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./firstBC]
    type = DirichletBC
    variable = u
    boundary = 10
    value = 1
  [../]
  [./secondBC]
    type = DirichletBC
    variable = u
    boundary = 11
    value = 0
  [../]
  [./thirdBC]
    type = DirichletBC
    variable = u
    boundary = 12
    value = 0
  [../]
  [./fourthBC]
    type = DirichletBC
    variable = u
    boundary = 13
    value = 0.5
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
