[Mesh]
  type = GeneratedMesh
  dim = 2
	xmin = 0
	xmax = 1
	ymin = 0
	ymax = 1
	nx = 2
	ny = 2
	elem_type = QUAD4

  uniform_refine = 3
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'udiff uconv uie vdiff vconv vie'

  [./udiff]
    type = Diffusion
    variable = u
  [../]

  [./uconv]
    type = Convection
    variable = u
    x = 10
    y = 1
  [../]

  [./uie]
    type = TimeDerivative
    variable = u
  [../]

  [./vdiff]
    type = Diffusion
    variable = v
  [../]

  [./vconv]
    type = Convection
    variable = v
    x = -10
    y = 1
  [../]

  [./vie]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  active = 'uleft uright vleft vright'

  [./uleft]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./uright]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./vleft]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 1
  [../]

  [./vright]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 0
  [../]
[]

[Postprocessors]
  [./dt]
  	type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 7
  dt = 0.1

  [./Adaptivity]
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
    start_time = 0.2
    stop_time = 0.4
    print_changed_info = true
  [../]
[]

[Output]
  output_initial = true
  interval = 2
  exodus = true
  perf_log = true
[]
