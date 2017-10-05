[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  # This MeshModifier currently only works with ReplicatedMesh.
  # For more information, refer to #2129.
  parallel_type = replicated
[]

[MeshModifiers]
  [./createNewSidesetOne]
    type = AddSideSetsFromBoundingBox
    boundary_id_old = 'right'
    boundary_id_new = 11
    bottom_left = '-0.1 -0.1 0'
    top_right = '0.9 0.9 0'
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
  [./leftBC]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]
  [./rightBC]
    type = DirichletBC
    variable = u
    boundary = 11
    value = 0
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
