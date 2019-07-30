[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
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

  [./sbb1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '-1 -1 -1'
    top_right = '0 0 0'
  []
  [./sbb2]
    type = SubdomainBoundingBoxGenerator
    input = sbb1
    block_id = 2
    bottom_left = '0 -1 -1'
    top_right = '1 0 0'
  []
  [./sbb3]
    type = SubdomainBoundingBoxGenerator
    input = sbb2
    block_id = 3
    bottom_left = '-1 0 -1'
    top_right = '0 1 0'
  []
  [./sbb4]
    type = SubdomainBoundingBoxGenerator
    input = sbb3
    block_id = 4
    bottom_left = '0 0 -1'
    top_right = '1 1 0'
  []
  [./sbb5]
    type = SubdomainBoundingBoxGenerator
    input = sbb4
    block_id = 5
    bottom_left = '-1 -1 0'
    top_right = '0 0 1'
  []
  [./sbb6]
    type = SubdomainBoundingBoxGenerator
    input = sbb5
    block_id = 6
    bottom_left = '0 -1 0'
    top_right = '1 0 1'
  []
  [./sbb7]
    type = SubdomainBoundingBoxGenerator
    input = sbb6
    block_id = 7
    bottom_left = '-1 0 0'
    top_right = '0 1 1'
  []
  [./sbb8]
    type = SubdomainBoundingBoxGenerator
    input = sbb7
    block_id = 8
    bottom_left = '0 0 0'
    top_right = '1 1 1'
  []

  [./re0]
    type = RenameBlockGenerator
    input = sbb8
    old_block_id = '12345    1   2   3     4'
    new_block_name = 'nill  one two three four'
  []

  [./re1]
    type = RenameBlockGenerator
    old_block_id = '12345    1          2'
    new_block_name = 'nill  one_and_two one_and_two'
    input = re0
  []

  [./does_nothing_there_is_no_block_2_now]
    type = RenameBlockGenerator
    old_block_id = 2
    new_block_id = 9
    input = re1
  []

  [./re2]
    type = RenameBlockGenerator
    old_block_id = '1 2     3 4 5 8'
    new_block_id = '1 12345 4 4 4 7'  # note this makes block_id=4 have name "three", since the first occurance of new_block_id=4 has name "three"
    input = does_nothing_there_is_no_block_2_now
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

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
