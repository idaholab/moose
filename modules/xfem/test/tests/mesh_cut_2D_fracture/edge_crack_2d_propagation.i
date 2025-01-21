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
    nx = 45
    ny = 15
    xmin = -1
    xmax = 0.49
    ymin = 0.0
    ymax = 1.0
    elem_type = QUAD4
  []
  [dispBlock]
    type = BoundingBoxNodeSetGenerator
    new_boundary = pull_set
    bottom_left = '-0.1 0.99 0'
    top_right = '0.1 1.01 0'
    input = gen
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y'
  crack_front_points_provider = cut_mesh2
  2d = true
  number_points_from_provider = 2
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.15'
  radius_outer = '0.45'
  poissons_ratio = 0.3
  youngs_modulus = 207000
  block = 0
  incremental = true
  used_by_xfem_to_grow_crack = true
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    incremental = true
    planar_formulation = plane_strain
    add_variables = true
    generate_output = 'stress_xx stress_yy'
  []
[]

[Functions]
  [pull_func]
    type = ParsedFunction
    expression = 0.00025*(1+t)
  []
[]

[BCs]
  [top_y]
    type = FunctionDirichletBC
    boundary = pull_set
    variable = disp_y
    function = pull_func
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
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       superlu_dist                 NONZERO               1e-20'
  line_search = 'none'
  nl_abs_tol = 1e-7
  [Predictor]
    type = SimplePredictor
    scale = 1.0
  []
  # time control
  start_time = 0.0
  dt = 1.0
  end_time = 3
  max_xfem_update = 100
[]

[Outputs]
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh2
  []
  [console]
    type = Console
    output_linear = false
    output_nonlinear = false
  []
[]
