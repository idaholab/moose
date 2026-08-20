[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]
[XFEM]
  geometric_cut_userobjects = 'cut_mesh2'
  qrule = moment_fitting
  output_cut_plane = true
  use_crack_tip_enrichment =true
  crack_front_definition = crackFrontDefinition
  enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y'
  cut_off_boundary = enriched_interface
  use_AD=true
  block=2
[]
[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = square7_7.e
  []
  [enriched_domain]
    type = SubdomainBoundingBoxGenerator
    input = fmg
    block_id = 2
    bottom_left = '0.0 0.2 0'
    top_right = '0.6 0.8 0'
  []
  [nset_enriched]
    type = SideSetsBetweenSubdomainsGenerator
    input = enriched_domain
    primary_block = '2'
    paired_block = '1'
    new_boundary = 'enriched_interface'
  []
[]
[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI InteractionIntegralKII'
  crack_front_points_provider = cut_mesh2
  2d = true
  number_points_from_provider = 1
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.0'
  radius_outer = '0.25'
  youngs_modulus = 200000
  poissons_ratio = 0.3
  output_q = true
  output_vpp = false
  incremental = true
  use_automatic_differentiation =true
  enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y'
  enriched_subdomain_id = 2
[]
[UserObjects]
    [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = line0p2.e
    growth_increment = 0.1667
    ki_vectorpostprocessor = "II_KI_1"
    k_critical = 0
   # block=2
  []
  [esm]
    type = CrackTipNodeLayerSubdomainModifier
    crack_front_definition = crackFrontDefinition
    enriched_subdomain_id =2
    base_subdomain_id =1# 0.7
    num_node_layers=2
  []
  [side_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 2
    outer_subdomains = 1
    update_boundary_name = enriched_interface
    assign_outer_surface_sides = false
    execute_on = XFEM_SUBDOMAIN_MODIFIER
    execution_order_group = 1
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
    use_automatic_differentiation =true
  []
[]

[AuxKernels]
  [stress_xx]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [stress_yy]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [stress_xy]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [vonmises]
    type = ADRankTwoScalarAux
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
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200000
    poissons_ratio = 0.3
  []
  [enrich_strain]
    type = ADComputeCrackTipEnrichmentIncrementalStrain
    crack_front_definition = crackFrontDefinition
    enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y'
    block=2
  []
  [strain]
    type = ADComputeIncrementalStrain
     block=1
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = 'Newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  #line_search=none
  automatic_scaling = true
  [Quadrature]
    type = GAUSS
    order = SIXTH
  []

  #l_max_its = 50
  nl_max_its = 200

  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-6
  start_time = 0.0
  dt = 0.1
  end_time = 0.4
  max_xfem_update = 1
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
[Outputs]
  file_base = sub_en_2layer_test
  exodus = true
  csv = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
