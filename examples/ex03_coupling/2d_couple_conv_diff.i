[Mesh]
  file = square.e
  uniform_refine = 4
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
  active = 'left_convected right_convected left_diffused right_diffused'

  [./left_convected]
    type = DirichletBC
    variable = convected
    boundary = '1'
    value = 0
  [../]

  [./right_convected]
    type = DirichletBC
    variable = convected
    boundary = '2'
    value = 1
  [../]

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 1
  [../]
[]

[Executioner]
  type = Steady
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]


