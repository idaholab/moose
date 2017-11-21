[XFEM]
  geometric_cut_userobjects = 'line_seg_cut_uo'
  qrule = volfrac
  output_cut_plane = true
  use_crack_tip_enrichment = true
  crack_front_definition = crack_tip
  enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y'
  displacements = 'disp_x disp_y'
  cut_off_boundary = all
  cut_off_radius = 0.2
[]

[UserObjects]
  [./line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '0.0 1.0 0.5 1.0'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
  [./crack_tip]
    type = CrackFrontDefinition
    crack_direction_method = CrackDirectionVector
    crack_front_points = '0.5 1.0 0'
    crack_direction_vector = '1 0 0'
    2d = true
    axis_2d = 2
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 9
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 2.0
  elem_type = QUAD4
[]

[MeshModifiers]
  [./all_node]
    type = BoundingBoxNodeSet
    new_boundary = 'all'
    top_right = '1 2 0'
    bottom_left = '0 0 0'
  [../]
  [./right_bottom_node]
    type = AddExtraNodeset
    new_boundary = 'right_bottom_node'
    coord = '1.0 0.0'
  [../]
  [./right_top_node]
    type = AddExtraNodeset
    new_boundary = 'right_top_node'
    coord = '1.0 2.0'
  [../]
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
 [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vonmises]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  [../]
  [./vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = vonmises
    scalar_type = vonmisesStress
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./top_y]
    type = Pressure
    variable = disp_y
    boundary = top
    component = 1
    factor = -1
  [../]
  [./bottom_y]
    type = Pressure
    variable = disp_y
    boundary = bottom
    component = 1
    factor = -1
  [../]
  [./fix_y]
    type = PresetBC
    boundary = right_bottom_node
    variable = disp_y
    value = 0.0
  [../]
  [./fix_x]
    type = PresetBC
    boundary = right_bottom_node
    variable = disp_x
    value =  0.0
  [../]
  [./fix_x2]
    type = PresetBC
    boundary = right_top_node
    variable = disp_x
    value =  0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeCrackTipEnrichmentSmallStrain
    displacements = 'disp_x disp_y'
    crack_front_definition = crack_tip
    enrichment_displacements = 'enrich1_x enrich2_x enrich3_x enrich4_x enrich1_y enrich2_y enrich3_y enrich4_y'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  # Since we do not sub-triangularize the tip element,
  # we need to use higher order quadrature rule to improve
  # integration accuracy.
  # Here second = SECOND is for regression test only.
  # However, order = SIXTH is recommended.
  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]

  # controls for linear iterations
  l_max_its = 10
  l_tol = 1e-4

  # controls for nonlinear iterations
  nl_max_its = 100
  nl_rel_tol = 1e-12 #11
  nl_abs_tol = 1e-12 #12

  # time control
  start_time = 0.0
  dt = 1.0
  end_time = 1.0
  dtmin = 1.0
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]


[Outputs]
  file_base = edge_crack_2d_out
  exodus = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
