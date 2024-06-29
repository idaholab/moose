[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh2'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 60
    ny = 10
    xmin = -3
    xmax = 3
    ymin = 0.0
    ymax = 1.0
    elem_type = QUAD4
  []
  [top_left]
    type = BoundingBoxNodeSetGenerator
    new_boundary = pull_top_left
    bottom_left = '-3.01 0.99 0'
    top_right = '-2.99 1.01 0'
    input = gen
  []
  [top_mid_left]
    type = BoundingBoxNodeSetGenerator
    new_boundary = pull_mid_left
    bottom_left = '-1.01 0.99 0'
    top_right = '-0.99 1.01 0'
    input = top_left
  []
  [top_mid_right]
    type = BoundingBoxNodeSetGenerator
    new_boundary = pull_mid_right
    bottom_left = '0.99 0.99 0'
    top_right = '1.01 1.01 0'
    input = top_mid_left
  []
  [top_right]
    type = BoundingBoxNodeSetGenerator
    new_boundary = pull_top_right
    bottom_left = '2.99 0.99 0'
    top_right = '3.01 1.01 0'
    input = top_mid_right
  []

  [top_mid_left_ss]
    type = SideSetsFromBoundingBoxGenerator
    input = top_right
    bottom_left = '-2.21 0.89 0'
    top_right = '-1.79 1.01 0'
    boundary_new = top_mid_left_ss
    boundaries_old = top
  []
  [top_mid_ss]
    type = SideSetsFromBoundingBoxGenerator
    input = top_mid_left_ss
    bottom_left = '-0.21 0.89 0'
    top_right = '0.21 1.01 0'
    boundary_new = top_mid_ss
    boundaries_old = top
  []
  [top_mid_right_ss]
    type = SideSetsFromBoundingBoxGenerator
    input = top_mid_ss
    bottom_left = '1.79 0.89 0'
    top_right = '2.21 1.01 0'
    boundary_new = top_mid_right_ss
    boundaries_old = top
  []

  [nucleation_strip]
    # strip in middle of domain where cracks can nucleate
    type = ParsedSubdomainMeshGenerator
    input = top_mid_right_ss
    combinatorial_geometry = 'y > 0.39 & y < 0.51'
    block_id = 10
  []
[]

[DomainIntegral]
  integrals = 'InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y'
  crack_front_points_provider = cut_mesh2
  2d = true
  number_points_from_provider = 0
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.15'
  radius_outer = '0.45'
  poissons_ratio = 0.3
  youngs_modulus = 207000
  block = 0
  incremental = false
  used_by_xfem_to_grow_crack = true
[]

[UserObjects]
  [nucleate]
    type = MeshCut2DRankTwoTensorNucleation
    tensor = stress
    scalar_type = MaxPrincipal
    nucleation_threshold = nucleation_threshold
    # initiate_on_boundary = 'left right'
    nucleation_radius = .41
    nucleation_length = 0.21
  []
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = make_edge_crack_in.e
    k_critical = 230
    growth_increment = 0.11
    nucleate_uo = nucleate
    execute_on = 'XFEM_MARK'
  []
[]
[AuxVariables]
  [nucleation_threshold]
    order = CONSTANT
    family = MONOMIAL
  []
[]
[ICs]
  [nucleation_bulk]
    type = ConstantIC
    value = 10000
    variable = nucleation_threshold
    block = 0
  []
  [nucleation_weak]
    type = FunctionIC
    function = nucleation_x
    variable = nucleation_threshold
    block = 10
  []
[]

[Functions]
  [nucleation_x]
    type = ParsedFunction
    expression = 'A*cos(pi*x)+D'
    symbol_names = 'A D'
    symbol_values = '100 200'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    planar_formulation = PLANE_STRAIN
    add_variables = true
  []
[]

[Functions]
  [bc_pull_edge]
    type = ParsedFunction
    expression = 0.0004*t
  []
  [bc_pull_mid]
    type = ParsedFunction
    expression = 0.0005*t
  []
[]

[BCs]
  [top_edge_nodes]
    type = FunctionDirichletBC
    boundary = 'pull_top_left pull_top_right'
    variable = disp_y
    function = bc_pull_edge
  []
  [top_mid_nodes]
    type = FunctionDirichletBC
    boundary = 'pull_mid_left pull_mid_right'
    variable = disp_y
    function = bc_pull_mid
  []
  # [top_middle]
  #   type = NeumannBC
  #   boundary = 'top_mid_left_ss top_mid_ss top_mid_right_ss'
  #   variable = disp_y
  #   value = -2000
  # []
  [top_middle]
    type = DirichletBC
    boundary = 'top_mid_left_ss top_mid_ss top_mid_right_ss'
    variable = disp_y
    value = 0
  []
  [bottom_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  []
  [bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'gmres lu superlu_dist'

  line_search = 'none'

  [Predictor]
    type = SimplePredictor
    scale = 1.0
  []

  reuse_preconditioner = true
  reuse_preconditioner_max_linear_its = 25

  # controls for linear iterations
  l_max_its = 100
  l_tol = 1e-2

  # controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9

  # time control
  start_time = 0.0
  dt = 1.0
  end_time = 10
  max_xfem_update = 100
[]

[Outputs]
  csv = true
  execute_on = FINAL
  # exodus = true
  # [xfemcutter]
  #   type = XFEMCutMeshOutput
  #   xfem_cutter_uo = cut_mesh2
  # []
  [console]
    type = Console
    output_linear = false
    output_nonlinear = false
  []
[]
