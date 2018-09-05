[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  nx = 4
  ny = 4
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
    boundary = 3
    value = 0.0
  [../]
  [./top]
    type = MTBC
    variable = u
    boundary = 1
    grad = 1.0
    prop_name = thermal_conductivity
  [../]
[]

[Materials]
  [./stateful]
    type = StatefulSpatialTest
    block = 0
  [../]
[]

[UserObjects]
  [./copy]
    type = MaterialCopyUserObject
    copy_times = 0.3
    copy_from_element = 0
    copy_to_element = 15
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Outputs]
  file_base = out_stateful_copy
  exodus = true
[]
