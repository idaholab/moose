[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
  use_crack_tip_enrichment = true
  crack_front_definition = crack_front
  enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y enrich1_z enrich2_z enrich3_z enrich4_z'
  cut_off_boundary = all
  cut_off_radius = 0.3
[]

[UserObjects]
  [circle_cut_uo]
    type = CircleCutUserObject
    cut_data = '0 0 0
                0.5 0 0
                0 0.5 0'
  []
  [crack_front]
    type = CrackFrontDefinition
    crack_direction_method = CurvedCrackFront
    crack_front_points = '0.500000000000000                   0                   0
                          0.000000000000000   0.500000000000000                   0
                         -0.500000000000000   0.000000000000000                   0
                         -0.000000000000000  -0.500000000000000                   0'
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 9
    ny = 9
    nz = 3
    xmin = -1.0
    xmax = 1.0
    ymin = -1.0
    ymax = 1.0
    zmin = -0.75
    zmax = 0.75
    elem_type = HEX8
  []
  [all_node]
    type = BoundingBoxNodeSetGenerator
    input = gen
    new_boundary = 'all'
    top_right = '1 1 1'
    bottom_left = '-1 -1 -1'
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
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [SED]
    order = CONSTANT
    family = MONOMIAL
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
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
[]

[Kernels]
  [TensorMechanics]
    use_displaced_mesh = false
    volumetric_locking_correction = false
  []
[]

[BCs]
  [top_z]
    type = Pressure
    variable = disp_z
    boundary = front
    factor = -1
  []
  [bottom_x]
    type = DirichletBC
    boundary = back
    variable = disp_x
    value = 0.0
  []
  [bottom_y]
    type = DirichletBC
    boundary = back
    variable = disp_y
    value = 0.0
  []
  [bottom_z]
    type = DirichletBC
    boundary = back
    variable = disp_z
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeCrackTipEnrichmentSmallStrain
    crack_front_definition = crack_front
    enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y enrich1_z enrich2_z enrich3_z enrich4_z'
  []
  [stress]
    type = ComputeLinearElasticStress
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

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  [Quadrature]
    type = GAUSS
    order = SECOND
  []

  # controls for linear iterations
  l_max_its = 10
  l_tol = 1e-2

  # controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12

  # time control
  start_time = 0.0
  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    output_linear = true
  []
[]
