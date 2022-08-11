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

#This is our new custom Convection Diffusion "Meta" block
#that adds multiple kernels into our simulation
#Convection and Diffusion kernels on the first variable
#Diffusion kernel on the second variable
#The Convection kernel is coupled to the Diffusion kernel on the second variable
[ConvectionDiffusion]
    variables = 'convected diffused'
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

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
