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
  [./sbb1]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '-1 -1 -1'
    top_right = '0 0 0'
  [../]
  [./sbb2]
    type = SubdomainBoundingBox
    block_id = 2
    bottom_left = '0 -1 -1'
    top_right = '1 0 0'
  [../]
  [./sbb3]
    type = SubdomainBoundingBox
    block_id = 3
    bottom_left = '-1 0 -1'
    top_right = '0 1 0'
  [../]
  [./sbb4]
    type = SubdomainBoundingBox
    block_id = 4
    bottom_left = '0 0 -1'
    top_right = '1 1 0'
  [../]
  [./sbb5]
    type = SubdomainBoundingBox
    block_id = 5
    bottom_left = '-1 -1 0'
    top_right = '0 0 1'
  [../]
  [./sbb6]
    type = SubdomainBoundingBox
    block_id = 6
    bottom_left = '0 -1 0'
    top_right = '1 0 1'
  [../]
  [./sbb7]
    type = SubdomainBoundingBox
    block_id = 7
    bottom_left = '-1 0 0'
    top_right = '0 1 1'
  [../]
  [./sbb8]
    type = SubdomainBoundingBox
    block_id = 8
    bottom_left = '0 0 0'
    top_right = '1 1 1'
  [../]

  [./re0]
    type = RenameBlock
    old_block_id = '12345    1   2   3     4'
    new_block_name = 'nill  one two three four'
    depends_on = 'sbb1 sbb2 sbb3 sbb4 sbb5 sbb6 sbb7 sbb8'
  [../]

  [./re1]
    type = RenameBlock
    old_block_id = '12345    1          2'
    new_block_name = 'nill  one_and_two one_and_two'
    depends_on = re0
  [../]

  [./does_nothing_there_is_no_block_2_now]
    type = RenameBlock
    old_block_id = 2
    new_block_id = 9
    depends_on = re1
  [../]

  [./re2]
    type = RenameBlock
    old_block_id = '1 2     3 4 5 8'
    new_block_id = '1 12345 4 4 4 7'  # note this makes block_id=4 have name "three", since the first occurance of new_block_id=4 has name "three"
    depends_on = does_nothing_there_is_no_block_2_now
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
  exodus = true
[]
