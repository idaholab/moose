[Mesh]
  file = mug.e
[]

[Variables]
  active = 'convected diffused'

  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff_convected conv diff_diffused'

  [./diff_convected]
    type = Diffusion
    variable = convected
  [../]

  [./conv]
    type = Convection
    variable = convected

    # Couple a variable into the convection kernel using local_name = simulationg_name syntax
    some_variable = diffused
  [../]

  [./diff_diffused]
    type = Diffusion
    variable = diffused
  [../]
[]

[BCs]
  active = 'bottom_convected top_convected bottom_diffused top_diffused'

  [./bottom_convected]
    type = DirichletBC
    variable = convected
    boundary = '1'
    value = 1
  [../]

  [./top_convected]
    type = DirichletBC
    variable = convected
    boundary = '2'
    value = 0
  [../]

  [./bottom_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 2
  [../]

  [./top_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]


