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
    type = ExampleConvection
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
  [./example_material]
    type = ExampleMaterial
    block = 1
    initial_diffusivity = 0.05
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 10
  dt = 1.0
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
