[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh2'
  qrule = moment_fitting
  output_cut_plane = true
  use_crack_tip_enrichment = true
  crack_front_definition = crackFrontDefinition
  enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y'
  displacements = 'disp_x disp_y'
  cut_off_boundary = all
  cut_off_radius = 0.3726
[]

[Mesh]
  file = square3_3.e
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y'
  crack_front_points_provider = cut_mesh2
  2d = true
  number_points_from_provider = 1
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.3'
  radius_outer = '0.5'
  youngs_modulus = 200000
  poissons_ratio = 0.3
  output_q = true
  output_vpp = false
  incremental = true
[]

[UserObjects]
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = line0p45.e
    growth_increment = 0.165
    ki_vectorpostprocessor = "II_KI_1"
    k_critical = 0
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [saved_x]
  []
  [saved_y]
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [vonmises]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = false
    displacements = 'disp_x disp_y'
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy'
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vonmises
    scalar_type = vonmisesStress
    execute_on = timestep_end
  []
[]

[Functions]
  [rampConstant]
    type = PiecewiseLinear
    x = '0. 1e-12 1.0'
    y = '1.0 1.0 1.0'
    scale_factor = -1e1
  []
[]

[BCs]
  [fix]
    type = DirichletBC
    boundary = 'top bottom fix'
    variable = disp_x
    value = 0.0
  []
  [fixy]
    type = DirichletBC
    boundary = 'fix'
    variable = disp_y
    value = 0.0
  []
  [Pressure]
    [Side1]
      boundary = 'top'
      function = rampConstant
    []
    [Side2]
      boundary = 'bottom'
      function = rampConstant
    []
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200000
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeCrackTipEnrichmentIncrementalStrain
    displacements = 'disp_x disp_y'
    crack_front_definition = crackFrontDefinition
    enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y'
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = 'Newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true
  [Quadrature]
    type = GAUSS
    order = SECOND
  []

  l_max_its = 50
  nl_max_its = 100

  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-5

  start_time = 0.0
  dt = 0.1
  end_time = 0.3
  max_xfem_update = 1
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
