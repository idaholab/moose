[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = gmg
    heights = '1'
    num_layers = '1'
    direction = '0 1 0'
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = '0'
    value = 0
  []
  [top]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
