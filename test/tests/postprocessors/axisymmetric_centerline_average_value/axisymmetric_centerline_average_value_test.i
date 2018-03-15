[Problem]
  coord_type = RZ
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 1
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'top bottom'

  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 0
  [../]

  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./average]
    type = AxisymmetricCenterlineAverageValue
    boundary = left
    variable = u
  [../]
[]

[Outputs]
  file_base = out
  exodus = true
[]
