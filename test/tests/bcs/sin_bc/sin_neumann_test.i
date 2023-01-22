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

#    [./InitialCondition]
#      type = FunctionIC
 #     function = initial_value
#    [../]
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
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./right]
    type = SinNeumannBC
    variable = u
    boundary = 1
    initial = 1.0
    final = 2.0
    duration = 10.0
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
