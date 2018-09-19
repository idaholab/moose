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

[AuxVariables]
  [./prop1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./prop1_dot]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./zero_dot]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./another_zero_dot]
    order = CONSTANT
    family = MONOMIAL
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

[AuxKernels]
  [./prop1_output]
    type = MaterialRealAux
    variable = prop1
    property = thermal_conductivity
  [../]

  [./prop1_dot_output]
    type = DotMaterialAux
    variable = prop1_dot
    property_name = thermal_conductivity
  [../]

  [./zero_dot_output]
    type = DotMaterialAux
    variable = zero_dot
  [../]

  [./another_zero_dot_output]
    type = DotMaterialAux
    variable = another_zero_dot
    property_name = 2
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
[]

[Postprocessors]
  [./integral]
    type = ElementAverageValue
    variable = prop1
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  l_max_its = 10
  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Outputs]
  file_base = out
  exodus = true
[]
