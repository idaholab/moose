[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  uniform_refine = 2
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
    prop_state = old # Use the "Old" value to compute conductivity
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
    type = StatefulTest
    prop_names = thermal_conductivity
    prop_values = 1.0
  [../]
[]

[Postprocessors]
  [./integral]
    type = ElementAverageValue
    variable = prop1
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  l_max_its = 10
  start_time = 0.0
  num_steps = 4
  dt = .1
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  marker = box
  [./Markers]
    [./box]
      type = BoxMarker
      bottom_left = '0.2 0.2 0.2'
      top_right = '0.4 0.4 0.4'
      inside = refine
      outside = coarsen
    [../]
  [../]
[]

[Outputs]
  exodus = true
  csv = true
[]
