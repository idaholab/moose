[Mesh]
  file = square.e
  uniform_refine = 4
[]

# This is our new custom Convection Diffusion "Meta" block
# that adds multiple kernels into our simulation
#
# Convection and Diffusion kernels on the first variable
# Diffusion kernel on the second variable
# The Convection kernel is coupled to the Diffusion kernel on the second variable
[ConvectionDiffusion]
    variables = 'convected diffused'
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
  type = Steady
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]
