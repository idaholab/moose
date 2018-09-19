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
  [./prop_dot]
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
  [./prop_dot_output]
    type = DotMaterialAux
    variable = prop_dot
    property_name = test_tensor
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
  [./rank2_tensor]
    type = GenericConstantRankTwoTensor
    tensor_values = '0 0 0 0 0 0 0 0 0'
    tensor_name = test_tensor
  [../]
[]

[Postprocessors]
  [./integral]
    type = ElementAverageValue
    variable = prop_dot
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
