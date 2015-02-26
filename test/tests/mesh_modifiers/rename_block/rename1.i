[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
[]

# Mesh Modifiers
[MeshModifiers]
  [./sbb0]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '-1 -1 -1'
    top_right = '0 0 0'
  [../]

  [./re_id]
    type = RenameBlock
    old_block_id = '0 1'
    new_block_id = '2 3'
  [../]
  [./ename_no_effect]
    type = RenameBlock
    old_block_id = '5 0 1'
    new_block_name = 'five zero one'
  [../]
  [./rename]
    type = RenameBlock
    old_block_id = '2'
    new_block_name = 'two_was_zero'
  [../]
  [./rename_block2]
    type = RenameBlock
    old_block_name = 'two_was_zero'
    new_block_name = 'simply_two'
  [../]
  [./rename_blockID_3]
    type = RenameBlock
    old_block_id = '3'
    new_block_name = 'three'
  [../]
  [./three_to_4]
    type = RenameBlock
    old_block_name = 'three'
    new_block_id = 4
  [../]
  [./another_no_effect]
    type = RenameBlock
    old_block_id = 3
    new_block_name = 'there_is_no_block_id_3_now'
  [../]
  [./should_not_do_anything]
    type = RenameBlock
    old_block_name = 'five'
    new_block_id = '4'
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
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
