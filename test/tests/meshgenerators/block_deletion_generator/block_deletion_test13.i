[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
  []

  [SubdomainBoundingBox]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '3 3 3'
  []

  [rename]
    type = RenameBlockGenerator
    input = SubdomainBoundingBox
    old_block = 1
    new_block = 'my_name'
  []

  [ed0]
    type = BlockDeletionGenerator
    input = rename
    block = 'my_name'
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
    boundary = bottom
    value = 1
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 10
  dt = 10

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
