[GlobalParams]
  displacements = 'disp_x disp_y'
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
  nx = 10
  ny = 20
  xmin = 0
  xmax = 1.0
  ymin = 0.0
  ymax = 2.0
  elem_type = QUAD4
[]

[]

[DomainIntegral]
  integrals = 'InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y'
  crack_front_points_provider = cut_mesh2
  2d=true
  number_points_from_provider = 0
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.15'
  radius_outer = '0.45'
  poissons_ratio = 0.0
  youngs_modulus = 207000
  block = 0
  incremental = true
[]

[AuxVariables]
[strength]
  order = CONSTANT
  family = MONOMIAL
[]
[]

[ICs]
[strength]
  type = VolumeWeightedWeibull
  variable = strength
  reference_volume = 1e-2
  weibull_modulus = 1
  median = 5000
[]
[]

[UserObjects]
  [nucleate]
    type = MeshCut2DRankTwoTensorNucleation
    tensor = stress
    scalar_type = MaxPrincipal
    nucleation_threshold = strength
    nucleation_radius = .21
    edge_extension_factor = .1
  []
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = make_edge_crack_in.e
    k_critical=500000 #Large so that cracks will not grow
    growth_increment = 0.11
    nucleate_uo = nucleate
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    planar_formulation = plane_strain
    add_variables = true
    generate_output = 'stress_xx stress_yy vonmises_stress max_principal_stress'
  []
[]

[Functions]
  [bc_pull_top]
    type = ParsedFunction
    expression = 0.0005*t
  []
[]

[BCs]
  [top_edges]
      type = FunctionDirichletBC
      boundary = 'top'
      variable = disp_y
      function = bc_pull_top
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
    poissons_ratio = 0.0
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

  l_max_its = 100
  l_tol = 1e-2

  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9

  start_time = 0.0
  dt = 1.0
  end_time = 5
  max_xfem_update = 1
[]

[Outputs]
  csv=true
  execute_on = final
  # exodus=true
  # [xfemcutter]
  #   type=XFEMCutMeshOutput
  #   xfem_cutter_uo=cut_mesh2
  # []
[]
