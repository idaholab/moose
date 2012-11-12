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
  active = 'udiff uconv uie vdiff vconv vie'

  [./udiff]
    type = Diffusion
    variable = u
  [../]

  [./uconv]
    type = Convection
    variable = u
    velocity = '10 1 0'
  [../]

  [./uie]
    type = ImplicitEuler
    variable = u
  [../]

  [./vdiff]
    type = Diffusion
    variable = v
  [../]

  [./vconv]
    type = Convection
    variable = v
    velocity = '-10 1 0'
  [../]

  [./vie]
    type = ImplicitEuler
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
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
    error_estimator = PatchRecoveryFooBar   # This is a bad error estimator
  [../]
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


