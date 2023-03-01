[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
  uniform_refine = 4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE_VEC
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE_VEC
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = VectorDiffusion
    variable = u
  [../]
[]

[DiracKernels]
  active = 'point_source1 point_source2'
  [./point_source1]
    type = VectorConstantPointSource
    variable = u
    values = '1.0 1.0 1.0'
    point = '0.2 0.3'
  [../]
  [./point_source2]
    type = VectorConstantPointSource
    variable = u
    values = '-0.5 -0.5 -0.5'
    point = '0.2 0.8'
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = VectorDirichletBC
    variable = u
    boundary = 3
    values = '0 0 0'
  [../]

  [./right]
    type = VectorDirichletBC
    variable = u
    boundary = 1
    values = '1 1 1'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = vector_2d_out
  exodus = true
[]
