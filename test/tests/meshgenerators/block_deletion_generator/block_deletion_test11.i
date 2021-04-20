[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = pyramid.e
  []

  [sbb2]
    type = SubdomainBoundingBoxGenerator
    input = fmg
    block_id = 2
    bottom_left = '-0.5 -0.5 -0.5'
    top_right = '0.5 0.5 0.5'
  []
  [swiss_cheese2]
    type = BlockDeletionGenerator
    block = 2
    input = 'sbb2'
  []
  [sbb3]
    type = SubdomainBoundingBoxGenerator
    input = swiss_cheese2
    block_id = 3
    bottom_left = '-5 -5 -3'
    top_right = '-2 -2 -1'
  []
  [swiss_cheese3]
    type = BlockDeletionGenerator
    block = 3
    input = 'sbb3'
  []
  [sbb4]
    type = SubdomainBoundingBoxGenerator
    input = swiss_cheese3
    block_id = 4
    bottom_left = '-1 2 -2'
    top_right = '1 5 0'
  []
  [swiss_cheese4]
    type = BlockDeletionGenerator
    block = 4
    input = 'sbb4'
  []
  [sbb5]
    type = OrientedSubdomainBoundingBoxGenerator
    input = swiss_cheese4
    block_id = 5
    center = '2.4 -1.4 0.4'
    height = 3
    length = 8
    length_direction = '-2 1 -1'
    width = 3
    width_direction = '1 2 0'
  []
  [swiss_cheese5]
    type = BlockDeletionGenerator
    block = 5
    input = 'sbb5'
  []
  [sbb6]
    type = OrientedSubdomainBoundingBoxGenerator
    input = swiss_cheese5
    block_id = 6
    center = '-1 0.4 2.2'
    height = 1
    length = 8
    length_direction = '2 -1 -1'
    width = 1
    width_direction = '1 2 0'
  []
  [swiss_cheese6]
    type = BlockDeletionGenerator
    block = 6
    input = 'sbb6'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 100
  dt = 100

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
