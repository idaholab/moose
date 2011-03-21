[Mesh]
  dim = 2
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
    diffusivity = 0.1
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
    type = ExampleTimeDerivative
    variable = diffused
    time_coefficient = 20.0
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

[Executioner]
  type = Transient   # Here we use the Transient Executioner
  perf_log = true
  petsc_options = '-snes_mf_operator'

  num_steps = 20
  dt = 0.5

  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
