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
  active = 'convected_ie example_diff conv diffused_ie diff'

  [./convected_ie]
    type = TimeDerivative
    variable = convected
  [../]

  [./example_diff]
    # This Kernel uses "diffusivity" from the active material
    type = ExampleDiffusion
    variable = convected
  [../]

  [./conv]
    type = Convection
    variable = convected
    some_variable = diffused
  [../]

  [./diffused_ie]
    type = TimeDerivative
    variable = diffused
  [../]

  [./diff]
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

    some_var = diffused
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

[Materials]
  active = example_material

  [./example_material]
    type = ExampleMaterial
    block = 1
    initial_diffusivity = 0.05
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'

  num_steps = 10
  dt = 1.0
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]


