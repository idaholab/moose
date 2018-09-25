[Mesh]
  dim = 3
  file = cube.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./heat]
    type = MatDiffusionTest
    variable = u
    prop_name = thermal_conductivity
  [../]
  [./ie]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0.0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1.0
  [../]
[]

[Materials]
  [./function_material]
    type = GenericFunctionMaterial
    block = 1
    prop_names = thermal_conductivity
    prop_values = '1.0+0.01*t+0.001*t*t'
  [../]
  [./error]
    type = DotErrorMaterial
    property_name = dummy
  [../]
[]

[Executioner]
  type = Transient
  l_max_its = 10
  start_time = 0.0
  num_steps = 5
  dt = .1
[]
