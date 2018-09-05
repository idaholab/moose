# This test creates several unused stateful properties.
# It's here to make sure that we don't consume too much
# memory if we store them all. With 180x180 elements
# we were previously seeing nearly a Gigabyte of memory
# consumed using TBB's map. We are now using unordered
# map which saves us 6x to 8x on memory.

[Mesh]
  type = GeneratedMesh
  nx = 10 #180
  ny = 10 #180
  dim = 2
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
[]

[Kernels]
  [./heat]
    type = MatDiffusionTest
    variable = u
    prop_name = thermal_conductivity
    prop_state = 'old'                  # Use the "Old" value to compute conductivity
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

  [./prop1_output_init]
    type = MaterialRealAux
    variable = prop1
    property = thermal_conductivity
    execute_on = initial
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0.0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1.0
  [../]
[]

[Materials]
  [./stateful1]
    type = StatefulTest
    prop_names = 'thermal_conductivity'
    prop_values = '1'
  [../]
  [./stateful2]
    type = StatefulTest
    prop_names = 'foo2'
    prop_values = '2'
  [../]
  [./stateful3]
    type = StatefulTest
    prop_names = 'foo3'
    prop_values = '3'
  [../]
  [./stateful4]
    type = StatefulTest
    prop_names = 'foo4'
    prop_values = '4'
  [../]
  [./stateful5]
    type = StatefulTest
    prop_names = 'foo5'
    prop_values = '5'
  [../]
  [./stateful6]
    type = StatefulTest
    prop_names = 'foo6'
    prop_values = '6'
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

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  l_max_its = 10
  start_time = 0.0
  num_steps = 1
  dt = .1
[]

[Outputs]
  exodus = true
[]
