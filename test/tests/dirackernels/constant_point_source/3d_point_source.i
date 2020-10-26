[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./point_source1]
    type = ConstantPointSource
    variable = u
    value = 0.1
    point = '0.2 0.3 0.0'
  [../]
  [./point_source2]
    type = ConstantPointSource
    variable = u
    value = -0.1
    point = '0.2 0.8 0.0'
  [../]
  [./point_source3]
    type = ConstantPointSource
    variable = u
    value = -1.0
    point = '0.8 0.5 0.8'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

[]

[Outputs]
  file_base = 3d_out
  exodus = true
[]
