[Mesh]
  dim = 3
  file = cube.e
  # This input file uses CONSTANT MONOMIAL AuxVariables, which don't
  # currently work right with ParallelMesh in parallel.  See #2122 for
  # more information.
  distribution = serial
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
    type = MatDiffusion
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
    block = 1
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  l_max_its = 10
  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Output]
  file_base = out
  output_initial = true
  exodus = true
  perf_log = true
  postprocessor_csv = true
[]

