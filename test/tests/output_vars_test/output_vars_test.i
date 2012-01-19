[Mesh]
  type = GeneratedMesh
	dim = 2
	xmin = 0
	xmax = 1
	ymin = 0
	ymax = 1
	nx = 10
	ny = 10
	elem_type = QUAD9
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff_u conv_u diff_v'

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./conv_u]
    type = CoupledForce
    variable = u
    v = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  active = 'left_u right_u left_v'

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 9
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 5
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 2
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
  output_variables = 'u'
[]
