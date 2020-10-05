[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
  uniform_refine = 3
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 0.1
      y1 = 0.1
      x2 = 0.6
      y2 = 0.6
      inside = 2.3
      outside = 4.6
      int_width = 0.2
    [../]
  [../]
[]

[AuxVariables]
  active = 'u_aux'

  [./u_aux]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 0.1
      y1 = 0.1
      x2 = 0.6
      y2 = 0.6
      inside = 1.34
      outside = 6.67
      int_width = 0.2
    [../]
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
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
