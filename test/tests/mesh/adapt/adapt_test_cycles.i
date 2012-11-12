[Mesh]
  dim = 2
  file = square.e
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
  active = 'udiff uconv uie vdiff'

  [./udiff]
    type = Diffusion
    variable = u
  [../]

  [./uconv]
    type = Convection
    variable = u
    velocity = '20 1 0'
  [../]

  [./uie]
    type = ImplicitEuler
    variable = u
  [../]

  [./vdiff]
    type = Diffusion
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

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'

  start_time = 0.0
  num_steps = 2
  dt = .1

  [./Adaptivity]
    refine_fraction = 0.3
    max_h_level = 7
    cycles_per_step = 2
  [../]
[]

[Output]
  file_base = out_cycles
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


