[Mesh]
  dim = 2
  file = square.e
  uniform_refine = 4
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'ConvectionDiffusion'

  # This is our new custom Convection Diffusion "Meta" block
  # that adds multiple kernels into our simulation
  #
  # Convection and Diffusion kernels on the first variable
  # Diffusion kernel on the second variable
  # The Convection kernel is coupled to the Diffusion kernel on the second variable
  [./ConvectionDiffusion]
    variables = 'u v'
  [../]
[]

[BCs]
  active = 'left_u right_u left_v right_v'

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = '2'
    value = 1

    some_var = v
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = '1'
    value = 0
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = '2'
    value = 1
  [../]

[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
