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
  nx = 20
  ny = 10
  xmin = 0
  xmax = 2
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD4
[]
[top_left]
  type = BoundingBoxNodeSetGenerator
  new_boundary = pull_top_left
  bottom_left = '-0.01 0.99 0'
  top_right = '0.11 1.01 0'
  input = gen
[]
[top_right]
  type = BoundingBoxNodeSetGenerator
  new_boundary = pull_top_right
  bottom_left = '1.89 0.99 0'
  top_right = '2.01 1.01 0'
  input = top_left
[]
[top_middle_ss]
  type = SideSetsFromBoundingBoxGenerator
  input = top_right
  bottom_left = '0.79 0.89 0'
  top_right = '1.21 1.01 0'
  block_id = '0'
  boundary_new = top_middle_ss
  boundaries_old = top
[]
[nucleate]
  type = ParsedSubdomainMeshGenerator
  input = top_middle_ss
  combinatorial_geometry = 'y > 0.39 & y < 0.51'
  block_id = 10
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
  poissons_ratio = 0.3
  youngs_modulus = 207000
  block = 0
  incremental = true
  used_by_xfem_to_grow_crack = true
[]

[UserObjects]
  [nucleate]
    type = MeshCut2DRankTwoTensorNucleation
    tensor = stress
    scalar_type = MaxPrincipal
    nucleation_threshold = nucleation_threshold
    initiate_on_boundary = 'left right'
    nucleation_length = .2
  []
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = make_edge_crack_in.e
    k_critical=230
    growth_increment = 0.11
    nucleate_uo = nucleate
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
    expression = '300+x*50'
  []
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    planar_formulation = plane_strain
    add_variables = true
    generate_output = 'stress_xx stress_yy vonmises_stress max_principal_stress'
  [../]
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
      boundary = 'pull_top_left pull_top_right'
      variable = disp_y
      function = bc_pull_top
  []
  [top_middle]
    type = NeumannBC
    boundary = top_middle_ss
    variable = disp_y
    value = -2000
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
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
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
  end_time = 5
  max_xfem_update = 100
[]

[Outputs]
  csv=true
  execute_on = TIMESTEP_END
  # [xfemcutter]
  #   type=XFEMCutMeshOutput
  #   xfem_cutter_uo=cut_mesh2
  # []
  # console = false
  [./console]
    type = Console
    output_linear = false
    output_nonlinear = false
  [../]
[]
