[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 15
  ny = 15
  nz = 15
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 100
  zmin = 0
  zmax = 100
  elem_type = HEX8
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./c]
    type = MultiSmoothCircleIC
    variable = c
    invalue = 1.0
    outvalue = 0.0001
    bubspac = 30.0 # This spacing is from bubble center to bubble center
    numbub = 6
    radius = 10.0
    int_width = 12.0
    radius_variation = 0.2
    radius_variation_type = uniform
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = c
  [../]
  [./diff]
    type = MatDiffusion
    variable = c
    diffusivity = D_v
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y z'
    [../]
  [../]
[]

[Materials]
  [./Dv]
    type = GenericConstantMaterial
    prop_names = D_v
    prop_values = 0.074802
  [../]
[]

[Postprocessors]
  [./bubbles]
    type = FeatureFloodCount
    variable = c
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -mat_mffd_type'
  petsc_options_value = 'hypre boomeramg 101 ds'
  l_max_its = 20
  l_tol = 1e-4
  nl_max_its = 20
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-11
  start_time = 0.0
  num_steps = 1
  dt = 100.0
  [./Adaptivity]
    refine_fraction = .5
  [../]
[]

[Outputs]
  exodus = true
[]
