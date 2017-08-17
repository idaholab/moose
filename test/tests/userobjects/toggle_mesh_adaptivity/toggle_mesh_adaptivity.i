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
    type = Diffusion
    variable = u
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.1
[]

[Adaptivity]
  cycles_per_step = 1
  marker = marker
  max_h_level = 2
  [./Markers]
    [./marker]
      type = BoxMarker
      bottom_left = '0.35 0.25 0'
      top_right = '0.5 0.5 0'
      inside = refine
      outside = coarsen
    [../]
  [../]
[]

[UserObjects]
  [./mesh_adaptivity_off]
    type = ToggleMeshAdaptivity
    mesh_adaptivity = 'off'
  [../]
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    print_mesh_changed_info = true
  [../]
[]

