[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Functions]
  [./initial_value]
    type = ParsedFunction
    expression = 'x'
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = FunctionIC
      function = initial_value
    [../]
  [../]
[]

[Kernels]
  active = 'diff ie'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ie]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  active = 'left right top bottom'

  [./left]
    type = SinDirichletBC
    variable = u
    boundary = 3
    initial = 0.0
    final = 1.0
    duration = 10.0
  [../]

  [./right]
    type = SinDirichletBC
    variable = u
    boundary = 1
    initial = 1.0
    final = 0.0
    duration = 10.0
  [../]

  # Explicit Natural Boundary Conditions
  [./top]
    type = WeakGradientBC
    variable = u
    boundary = 2
  [../]

  [./bottom]
    type = WeakGradientBC
    variable = u
    boundary = 0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  num_steps = 10
  dt = 1.0
[]

[Outputs]
  exodus = true
[]
