[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  ymin = 0
  zmin = 0
  xmax = 10
  ymax = 10
  zmax = 10
  nx = 30
  ny = 30
  nz = 30
[]

[MeshModifiers]
  [./point]
    type = BoundingBoxNodeSet
    bottom_left = '5 5 5'
    top_right = '5 5 5'
    new_boundary = 999
  [../]
[]

[GlobalParams]
  displacements= 'u_x u_y u_z'
  volumetric_locking_correction = true
  op_num = 25
  var_name_base = gr
[]

[UserObjects]
  [./euler_angle_file]
    type = ConstWeightEulerAngleProvider
    file_name = Poly120.ori
  [../]
  [./voronoi]
    type = PolycrystalVoronoi
    file_name = Poly120.txt
  [../]
  [./grain_tracker]
   type = GrainTrackerElasticity
   threshold = 0.2
   connecting_threshold = 0.08
   remap_grains = true
   euler_angle_provider = euler_angle_file
   C_ijkl = '1.27 0.708 0.708 1.27 0.708 1.27 0.7355 0.7355 0.7355'
   compute_var_to_feature_map = true
 [../]
 [./global_strain_uo]
    type = GlobalStrainUserObject
    applied_stress_tensor = '.5 0 0 0 0 0'
    execute_on = 'Initial Linear Nonlinear'
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = voronoi
    [../]
  [../]
  #[./dispIC]
  #  type = FunctionIC
  #  variable = disp_x
  #  function = .05*x
  #[../]
  #[./disp_IC]
  #  type = VectorFunctionIC
  #  variable = disp_x
  #  function = disp_fun
  #[../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
  [./global_strain]
    order = SIXTH
    family = SCALAR
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
     [./all]
       strain = SMALL
       add_variables = true
       #generate_output = 'strain_xy strain_xx strain_yy'
       global_strain = global_strain
     [../]
    [../]
  [../]
[]

[ScalarKernels]
  [./global_strain]
    type = GlobalStrain
    variable = global_strain
    global_strain_uo = global_strain_uo
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
  #[./right_pull]
  #  type = PresetBC
  #  variable = u_x
  #  boundary = right
  #  value = .005
  #[../]
  [./point_anchor_x]
    type = PresetBC
    variable = u_x
    boundary = 999
    value = 0.0
  [../]

  #[./point_anchor_y]
  #  type = PresetBC
  #  variable = u_y
  #  boundary = 999
  #  value = 0.0
  #[../]

  #[./left_fix]
  #  type = PresetBC
  #  variable = u_x
  #  boundary = left
  #  value = 0
  #[../]
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./strain_xx]
    family = MONOMIAL
    order = FIRST
  [../]
  [./strain_xy]
    family = MONOMIAL
    order = FIRST
  [../]
  [./strain_xz]
    family = MONOMIAL
    order = FIRST
  [../]
  [./strain_yy]
    family = MONOMIAL
    order = FIRST
  [../]

[]

[AuxKernels]
  [./disp_x]
    type = GlobalDisplacementAux
    variable = disp_x
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
    component = 0
  [../]
  [./disp_y]
    type = GlobalDisplacementAux
    variable = disp_y
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
    component = 1
  [../]
  [./disp_z]
    type = GlobalDisplacementAux
    variable = disp_z
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
    component = 2
  [../]
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./strain_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xz
    index_i = 0
    index_j = 1
  [../]
  [./strain_xz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xz
    index_i = 0
    index_j = 2
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  [../]
[]

[Materials]
  # [./ElasticityTensor]
  #   type = ComputeBlockRotatedElasticityTensor
  #   euler_angle_provider = euler_angle_file
  #   fill_method = symmetric9
  #   C_ijkl = '1.27 0.708 0.708 1.27 0.708 1.27 0.7355 0.7355 0.7355'
  #   # C_ijkl = '1.27e5 0.708e5 0.708e5 1.27e5 0.708e5 1.27e5 0.7355e5 0.7355e5 0.7355e5'
  #   offset = 0
  # [../]
  [./elasticity]
    type = ComputePolycrystalElasticityTensor
    grain_tracker = grain_tracker
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./global_strain]
    type = ComputeGlobalStrain
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 31 0.7'
  l_tol = 1.0e-3
#  l_max_its = 30
#  nl_max_its = 40
  nl_rel_tol = 1.0e-7
  #start_time = 0.0
  #num_steps = 25
  #dt = 0.01

 [./Predictor]
   type = SimplePredictor
   scale = .5
  [../]
[]

[Postprocessors]
  #[./strain_x]
  #  type = SideAverageValue
  #  variable = strain_xx
  #  boundary = left
  #[../]
  #[./stress_x]
  #  type = SideAverageValue
  #  variable = stress_xx
  #  boundary = left
  #[../]
  [./dis_x]
    type = SideAverageValue
    variable = disp_x
    boundary = left
  [../]
  [./memory]
    type = MemoryUsage
    mem_type = physical_memory
    mem_units = mebibytes
    execute_on = 'NONLINEAR LINEAR TIMESTEP_END'
    report_peak_value = true
  [../]
[]

[VectorPostprocessors]
  #[./stress_strain]
  #  type = VectorOfPostprocessors
  #  postprocessors = 'strain_x stress_x dis_x'
  #[../]
  [./top_visual]
    type = SideValueSampler
    variable = 'strain_xx strain_yy strain_xy strain_xz disp_x u_x disp_y u_y'
    sort_by = id
    execute_on = 'initial timestep_end'
    boundary = top
    use_displaced_mesh = false
    outputs = 'csv'
  [../]
[]

[Outputs]
  perf_graph = true
  csv = true
  exodus = true
[]
