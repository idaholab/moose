[Mesh]
  dim = 3
  file = cube.e
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./prop1]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./prop1_output]
    type = MaterialRealAux
    variable = prop1
    property = thermal_conductivity
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
  [./stateful]
    type = ComputingInitialTest
    block = 1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  l_max_its = 10
  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Outputs]
  file_base = computing_initial_residual_test_out
  [./out]
    type = Exodus
    elemental_as_nodal = true
    execute_elemental_on = none
  [../]
[]
