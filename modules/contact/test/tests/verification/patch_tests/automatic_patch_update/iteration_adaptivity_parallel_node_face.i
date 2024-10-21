[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  coord_type = XYZ
  patch_update_strategy = iteration
  patch_size = 8
  ghosting_patch_size = 20
  [cube1]
    type = GeneratedMeshGenerator
    dim = 2
    boundary_name_prefix = cube1
    xmax = 1
    ymax = 1
    nx = 2
    ny = 2
  []
  [cube2]
    type = GeneratedMeshGenerator
    dim = 2
    boundary_name_prefix = cube2
    boundary_id_offset = 5
    xmax = 1
    ymax = 1
    nx = 2
    ny = 2
  []
  [block_id]
    type = SubdomainIDGenerator
    input = cube2
    subdomain_id = 2
  []
  [combine]
    inputs = 'cube1 block_id'
    type = CombinerGenerator
    positions = '0 0 0
                 0 1 0'
  []
  [rename2]
    type = RenameBlockGenerator
    input = combine
    old_block = '0 2'
    new_block = 'cube1 cube2'
  []
[]

[Adaptivity]
  initial_marker = box
  initial_steps = 1
  max_h_level = 1
  [Markers]
    [box]
      type = BoxMarker
      bottom_left = '0 0 0'
      top_right = '0.5 0.5 0'
      inside = refine
      outside = do_nothing
    []
  []
[]

[Variables]
  [disp_x]
    block = 'cube1 cube2'
  []
  [disp_y]
    block = 'cube1 cube2'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [cube1_mechanics]
    strain = FINITE
    block = 'cube1 cube2'
  []
[]

[BCs]
  [cube1_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'cube1_bottom '
    value = 0.0
  []
  [cube1_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'cube1_bottom '
    value = 0.0
  []
  [cube2_y]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 'cube2_top'
    function = '-t'
    preset = false
  []
  [cube2_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'cube2_top'
    value = 0
  []
[]

[Materials]
  [cube1_elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 68.9e9
    poissons_ratio = 0.3
    block = 'cube1'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = 'cube1 cube2'
  []
  [cube2_elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 140e9
    poissons_ratio = 0.3
    block = 'cube2'
  []
[]

[Contact]
  [contactswell]
    secondary = cube1_top
    primary = cube2_bottom
    model = frictionless
    formulation = kinematic
    penalty = 1.0e6
    normalize_penalty = true
    tangential_tolerance = 0.1
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       superlu_dist'

  line_search = 'none'

  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-16
  nl_max_its = 50

  l_tol = 1e-4
  l_max_its = 50

  start_time = 0.0
  end_time = 0.02e-3

  dtmax = 4
  dtmin = 0.001e-3
  dt = 0.01e-3

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  execute_on = 'FINAL'
[]
