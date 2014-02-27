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

[Executioner]
  type = Transient

  dt = 1
  start_time = 0
  end_time = 40
  num_steps = 1000
[]

[Outputs]
  file_base = out
  csv = true
  exodus = true
  sync_times = '10.5 20 30.5'
  [./console]
    type = Console
    perf_log = true
  [../]
[]
