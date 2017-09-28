[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
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
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 4
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  initial_marker = circle_marker
  initial_steps = 4
  marker = coarsen
  cycles_per_step = 4
  max_h_level = 2
  recompute_markers_during_cycles = true
  [./Markers]
    [./circle_marker]
      type = CircleMarker
      point = '0.6 0.1 0'
      radius = 0.2
      inside = refine
      outside = do_nothing
      periodic_variable = u
    [../]
    [./coarsen]
      type = UniformMarker
      mark = coarsen
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

