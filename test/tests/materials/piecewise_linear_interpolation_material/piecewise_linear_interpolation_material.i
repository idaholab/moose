[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  nz = 0
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff1]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./m1]
    type = PiecewiseLinearInterpolationMaterial
    property = m1
    variable = u
    xy_data = '0 0
               1 1'
    block = 0
    outputs = all
  [../]

  [./m2]
    type = PiecewiseLinearInterpolationMaterial
    property = m2
    variable = u
    x = '0 1'
    y = '0 1'
    block = 0
    outputs = all
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
