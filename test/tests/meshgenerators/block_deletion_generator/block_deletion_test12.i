[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
  []

  [mark]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0.9 0.9 0'
    top_right = '3.1 3.1 0'
  []
  [delete]
    type = BlockDeletionGenerator
    block = 1
    input = mark
    new_boundary = cut_surface
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
  [outer]
    type = DirichletBC
    variable = u
    boundary = 'top bottom left right'
    value = 1
  []
  [inner]
    type = DirichletBC
    variable = u
    boundary = cut_surface
    value = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
