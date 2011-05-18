[Mesh]
  [./Generation]
    dim = 2
  	xmin = 0
  	xmax = 1
  	ymin = 0
  	ymax = 1
  	nx = 4
  	ny = 4
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = MatDiffusion
    variable = u
    prop_name = some_prop
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Materials]
  # order is switched intentionally, so we won't get luck and dep-resolver has to do its job
  [./mat2]
    type = CoupledMaterial
    block = 0
    mat_prop = matp
  [../]

  [./mat1]
    type = MTMaterial
    block = 0
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
[]
