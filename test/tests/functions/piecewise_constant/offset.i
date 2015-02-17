[Mesh]
  file = cube.e
  # This problem only has 1 element, so using ParallelMesh in parallel
  # isn't really an option, and we don't care that much about ParallelMesh
  # in serial.
  distribution = serial
[]

[Variables]
  [./aVar]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
[]

[Functions]
  [./a]
    type = PiecewiseConstant
    x_offset = 0.1
    xy_data = '0 0.1
               1 0.2
               2 0.1'
    direction = left
  [../]
[]

[Kernels]
  [./diffa]
    type = Diffusion
    variable = aVar
  [../]
[]

[BCs]
  [./a]
    type = FunctionDirichletBC
    variable = aVar
    boundary = '1'
    function = a
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.2
  start_time = 0.1
  end_time = 3
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  output_on = 'initial timestep_end'
  [./console]
    type = Console
    perf_log = true
    output_on = 'timestep_end failed nonlinear'
  [../]
[]
