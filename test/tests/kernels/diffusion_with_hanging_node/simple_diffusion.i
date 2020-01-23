[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
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

[BCs]
  # BCs cannot be preset due to Jacobian test
  [./left]
    type = DirichletBC
    preset = false
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    preset = false
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Preconditioning]
  [./pre]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options = '-pc_svd_monitor -ksp_view_pmat'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'
[]

[Outputs]
  exodus = true
[]

[Adaptivity]
  marker = box
  max_h_level = 1
  initial_steps = 1

  [./Markers]
    [./box]
      type = BoxMarker
      bottom_left = '0.5 0 0'
      top_right = '1 1 0'
      inside = 'refine'
      outside = 'do_nothing'
    [../]
  [../]
[]
