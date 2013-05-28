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
  active = 'example_diff conv diff euler'

  [./example_diff]
    type = ExampleDiffusion
    variable = convected
  [../]

  [./conv]
    type = Convection
    variable = convected
    some_variable = diffused
  [../]

  [./diff]
    type = Diffusion
    variable = diffused
  [../]

  [./euler]
    type = ExampleImplicitEuler
    variable = diffused
  [../]
[]

[BCs]
  active = 'left_convected right_convected left_diffused right_diffused'

  [./left_convected]
    type = DirichletBC
    variable = convected
    boundary = 'left'
    value = 0
  [../]

  [./right_convected]
    type = DirichletBC
    variable = convected
    boundary = 'right'
    value = 1
  [../]

  [./left_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'left'
    value = 0
  [../]

  [./right_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'right'
    value = 1
  [../]

[]

[Materials]
  active = 'example'

  [./example]
    type = ExampleMaterial
    block = 1
    diffusivity = 0.5
    time_coefficient = 20.0
  [../]
[]

[Executioner]
  type = Transient

  petsc_options = '-snes_mf_operator'

  num_steps = 40

  # Use our custom TimeStepper
  [./TimeStepper]
    type = TransientHalf
    ratio = 0.5
    min_dt = 0.01
    dt = 1
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]


