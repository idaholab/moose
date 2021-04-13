[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 2
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
    zmin = 0
    zmax = 2
  []

  [SubdomainBoundingBox1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '1 2 1'
  []
  [SubdomainBoundingBox2]
    type = SubdomainBoundingBoxGenerator
    input = SubdomainBoundingBox1
    block_id = 1
    bottom_left = '1 1 0'
    top_right = '3 3 1'
  []
  [SubdomainBoundingBox3]
    type = SubdomainBoundingBoxGenerator
    input = SubdomainBoundingBox2
    block_id = 1
    bottom_left = '2 2 1'
    top_right = '3 3 2'
  []
  [ed0]
    type = BlockDeletionGenerator
    block = 1
    input = SubdomainBoundingBox3
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
