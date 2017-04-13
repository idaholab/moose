[Mesh]
  file = square.e
  uniform_refine = 4
[]

[Variables]
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
  [./example_diff]
    type = ExampleDiffusion
    variable = convected
  [../]

  [./conv]
    type = ExampleConvection
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
  [./example]
    type = ExampleMaterial
    block = 1
    diffusivity = 0.5
    time_coefficient = 20.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 12

  # Use our custom TimeStepper
  [./TimeStepper]
    type = TransientHalf
    ratio = 0.5
    min_dt = 0.01
    dt = 1
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
