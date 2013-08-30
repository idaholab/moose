# Two non-linear variables with the same name

[Mesh]
	type = GeneratedMesh
	dim = 2
	xmin = 0
	xmax = 1
	ymin = 0
	ymax = 1
	nx = 5
	ny = 5
	elem_type = QUAD9
[]

[Variables]
  [./u]
     order = FIRST
     family = LAGRANGE
  [../]

  # Note this section is a repeat of the one above
  [./u]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]
