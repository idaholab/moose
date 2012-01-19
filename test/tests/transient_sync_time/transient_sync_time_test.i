[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20

  x_min = 0
  x_max = 1

  y_min = 0
  y_max = 1
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
    value = sin(pi*0.1*x*t)
  [../]

  # Laplacian of the function above
  [./interior_func]
    type = ParsedFunction
    value = 0.01*pi*pi*t*t*sin(0.1*pi*x*t)
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = UserForcingFunction
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

[Materials]
  [./empty]
    type = EmptyMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Transient

  dt = 1
  start_time = 0
  end_time = 40
  num_steps = 1000

  # These times will be sync'd in the output
  sync_times = '10.5 20 30.5'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  postprocessor_csv = true
  perf_log = true
[]
