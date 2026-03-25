[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 10
    xmin = -2000
    xmax = 2000
    ymin = -2000
    ymax = 2000
    zmin = 0
    zmax = 500
  []
[]

[DiracKernels]
  [bh1]
    type = ConstantPointSource
    variable = u
    value = 10
    point = '0 0 201.5'
    point_not_found_behavior='WARNING'
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

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
