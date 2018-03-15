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
  num_steps = 6
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  initial_steps = 1
  initial_marker = two_circle_marker
  cycles_per_step = 1
  marker = two_circle_marker
  max_h_level = 1
  [./Markers]
    [./two_circle_marker]
      type = TwoCircleMarker
      point1 = '0.5 0.5 0'
      radius1 = 0.3
      point2 = '0.35 0.25 0'
      radius2 = 0.3
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

