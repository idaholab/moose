[Mesh]
  [./Generation]
  	dim = 2
  	xmin = 0
  	xmax = 1
  	ymin = 0
  	ymax = 1
  	nx = 2
  	ny = 2
  [../]
[]

[Variables]
#  active = 'u v'
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Preconditioning]
#  active = 'SingleMatrix'
  active = ' '

  [./SingleMatrix]
    preconditioner  = 'LU'
    off_diag_row    = 'u'
    off_diag_column = 'v'
  [../]
[]

[Kernels]
#  active = 'diff_u conv_v diff_v'
  active = 'diff_u'

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./conv_v]
    type = CoupledForce
    variable = v
    v = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
#  active = 'left_u right_u left_v'
  active = 'left_u right_u'

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

#  l_max_its = 1
#  nl_max_its = 1

#	nl_rel_tol = 1e-10

  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
