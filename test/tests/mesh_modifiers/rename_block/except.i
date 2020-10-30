[Mesh]
  active = 'gen sbb0'
  [gen]
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

  # Mesh Modifiers
  [sbb0]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '-1 -1 -1'
    top_right = '0 0 0'
  []

  [too_many_new_id]
    type = RenameBlockGenerator
    input = sbb0
    old_block_id = '0'
    new_block_id = '2 3'
  []
  [old_id_and_name]
    type = RenameBlockGenerator
    input = sbb0
    old_block_id = '0 1'
    old_block_name = 'zero one'
    new_block_id = '2 3'
  []
  [no_old_id]
    type = RenameBlockGenerator
    input = sbb0
    new_block_id = '2 3'
  []
  [too_many_old]
    type = RenameBlockGenerator
    input = sbb0
    old_block_id = '1 2 3'
    new_block_name = 'two three'
  []
  [new_id_and_name]
    type = RenameBlockGenerator
    input = sbb0
    old_block_id = '1 2 3'
    new_block_id = '5 6 7'
    new_block_name = 'five six seven'
  []
  [no_new]
    type = RenameBlockGenerator
    input = sbb0
    old_block_id = '1 2 3'
  []

[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
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
