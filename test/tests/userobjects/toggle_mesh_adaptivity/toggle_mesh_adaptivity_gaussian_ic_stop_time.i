[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./gaussian_ic]
    type = FunctionIC
    variable = u
    function = gaussian_2d
  [../]
[]

[Functions]
  [./gaussian_2d]
    type = ParsedFunction
    expression = exp(-((x-x0)*(x-x0)+(y-y0)*(y-y0))/2.0/sigma/sigma)
    symbol_names = 'sigma x0 y0'
    symbol_values = '0.05 0.35 0.25'
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.02
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      variable = u
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  initial_steps = 1
  initial_marker = marker
  cycles_per_step = 1
  marker = marker
  max_h_level = 2
  stop_time = 0.0
  [./Markers]
    [./marker]
      type = CircleMarker
      point = '0.35 0.25 0'
      radius = 0.2
      inside = refine
      outside = coarsen
    [../]
  [../]
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    print_mesh_changed_info = true
  [../]
[]

