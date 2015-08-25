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
  active = 'sbb0'
  [./sbb0]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '-1 -1 -1'
    top_right = '0 0 0'
  [../]

  [./too_many_new_id]
    type = RenameBlock
    old_block_id = '0'
    new_block_id = '2 3'
    depends_on = sbb0
  [../]
  [./old_id_and_name]
    type = RenameBlock
    old_block_id = '0 1'
    old_block_name = 'zero one'
    new_block_id = '2 3'
    depends_on = sbb0
  [../]
  [./no_old_id]
    type = RenameBlock
    new_block_id = '2 3'
    depends_on = sbb0
  [../]
  [./too_many_old]
    type = RenameBlock
    old_block_id = '1 2 3'
    new_block_name = 'two three'
    depends_on = sbb0
  [../]
  [./new_id_and_name]
    type = RenameBlock
    old_block_id = '1 2 3'
    new_block_id = '5 6 7'
    new_block_name = 'five six seven'
    depends_on = sbb0
  [../]
  [./no_new]
    type = RenameBlock
    old_block_id = '1 2 3'
    depends_on = sbb0
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


[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
