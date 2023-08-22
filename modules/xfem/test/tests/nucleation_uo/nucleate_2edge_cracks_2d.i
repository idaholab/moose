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
  nx = 30
  ny = 15
  xmin = -2
  xmax = -.2
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD4
[]
[dispBlock_top]
  type = BoundingBoxNodeSetGenerator
  new_boundary = pull_top_y
  bottom_left = '-2.1 0.99 0'
  top_right = '-1.9 1.01 0'
  input = gen
[]
[dispBlock_bot]
  type = BoundingBoxNodeSetGenerator
  new_boundary = pull_bot_y
  bottom_left = '-2.1 -.01 0'
  top_right = '-1.9 0.01 0'
  input = dispBlock_top
[]
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y'
  crack_front_points_provider = cut_mesh2
  2d=true
  number_points_from_provider = 1
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
  #fixme, nucleate has to be before cut_mesh2 in the input file or cut_mesh2 can't finde the nucleate_uo
  [nucleate]
    type = MeshCut2DRankTwoTensorNucleation
    tensor = stress
    scalar_type = MaxPrincipal
    nucleation_threshold = nucleation_threshold
    initiate_on_boundary = 'left bottom'
    average = true
    nucleation_length = .1
  []
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = make_edge_crack_in.e
    k_critical=80
    growth_increment = 0.1
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
   [nucleation]
     type = FunctionIC
     function =  nucleation_x_y
     variable = nucleation_threshold
   []
  # [nucleation]
  #   type = VolumeWeightedWeibull
  #   variable = nucleation_threshold
  #   reference_volume = 0.01 #This is the volume of an element for a 100x100 mesh
  #   weibull_modulus = 2
  #   median = 5000.0
  # []
[]

[Functions]
  [nucleation_y]
    type = ParsedFunction
    expression = 'if(y>0.7,10000,if(y<0.5,10000,4000*(1-y)^2-10000))'
  []
  [nucleation_x]
    type = ParsedFunction
    expression = 'if(x>-0.9,10000,if(x<-1.1,10000,1000*(x)^2-10000))'
  []
  [nucleation_x_y]
    type = LinearCombinationFunction
    functions = 'nucleation_x nucleation_y'
    w = '1 1'
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
    expression = 'if(t<6,0.0008*t,0.0008*5+0.0004*(t-6))'
  []
  [bc_pull_bot]
    type = ParsedFunction
    expression = 0.0004*t
  []
[]

[BCs]
  [top_left]
      type = FunctionDirichletBC
      boundary = pull_top_y
      variable = disp_y
      function = bc_pull_top
  []
  [bot_left]
    type = FunctionDirichletBC
    boundary = pull_bot_y
    variable = disp_y
    function = bc_pull_bot
[]
  [bottom_x]
    type = DirichletBC
    boundary = right
    variable = disp_x
    value = 0.0
  []
  [bottom_y]
    type = DirichletBC
    boundary = right
    variable = disp_y
    value = 0.0
  []
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
    block = 0
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = 0
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
  end_time = 55
  max_xfem_update = 2
[]

[Outputs]
  # csv=true
  exodus = true
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
