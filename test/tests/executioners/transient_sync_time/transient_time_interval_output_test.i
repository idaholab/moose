[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./bc_func]
    type = ParsedFunction
    expression = sin(pi*0.1*x*t)
  [../]

  # Laplacian of the function above
  [./interior_func]
    type = ParsedFunction
    expression = 0.01*pi*pi*t*t*sin(0.1*pi*x*t)
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = interior_func
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = bc_func
  [../]
[]

[Executioner]
  type = Transient

  dt = 1
  start_time = 0
  num_steps = 10

  # These times will be sync'd in the output
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_tio
  interval = 3
  [./exodus]
    type = Exodus
    execute_on = 'final timestep_end'
  [../]
[]
