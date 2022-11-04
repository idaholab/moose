[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 2
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.2
  elem_type = HEX8
[]

[UserObjects]
  [./cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_file = mesh_edge_crack.xda
    growth_dir_method = 'max_hoop_stress'
    size_control = 1
    n_step_growth = 1
    function_v = growth_func_v
    crack_front_nodes = '7 6 5 4'
  [../]
[]

[Functions]
  [./growth_func_v]
    type = ParsedFunction
    expression = 0.15
  [../]
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y disp_z'
  crack_front_points_provider = cut_mesh
  number_points_from_provider = 4
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.15'
  radius_outer = '0.45'
  poissons_ratio = 0.3
  youngs_modulus = 207000
  block = 0
  incremental = true
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
  [../]
[]

[Functions]
  [./top_trac_x]
    type = ConstantFunction
    value = 100
  [../]
  [./top_trac_y]
    type = ConstantFunction
    value = 0
  [../]
[]


[BCs]
  [./top_x]
    type = FunctionNeumannBC
    boundary = top
    variable = disp_x
    function = top_trac_x
  [../]
  [./top_y]
    type = FunctionNeumannBC
    boundary = top
    variable = disp_y
    function = top_trac_y
  [../]
  [./bottom_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    boundary = bottom
    variable = disp_z
    value = 0.0
  [../]
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
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 4.0
  max_xfem_update = 1
[]

[Outputs]
  file_base = edge_crack_3d_mhs_out
  execute_on = 'timestep_end'
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
