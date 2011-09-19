[Mesh]
  file = rect-2blk.e
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff_u diff_v'

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  active = 'left_u right_u left_v right_v'

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 6
    value = 0
  [../]

  [./right_u]
    type = NeumannBC
    variable = u
    boundary = 8
    value = 4
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 6
    value = 1
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 6
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = '1 2'
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out_vars
  output_initial = false
  interval = 1
  exodus = true
  perf_log = true
[]
