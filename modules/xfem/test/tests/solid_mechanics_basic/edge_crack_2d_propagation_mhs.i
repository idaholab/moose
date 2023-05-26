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
  xmin = -1.5
  xmax = 1.5
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
  2d=true
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

[UserObjects]
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = make_edge_crack_in.e
    k_critical=80
    growth_increment = 0.1
  []
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    planar_formulation = plane_strain
    add_variables = true
    generate_output = 'stress_xx stress_yy vonmises_stress'
  [../]
[]

[BCs]
  [top_y]
      type = DirichletBC
      boundary = pull_set
      variable = disp_y
      value = 0.001
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
  end_time = 1
  max_xfem_update = 100
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
  [xfemcutter]
    type=XFEMCutMeshOutput
    xfem_cutter_uo=cut_mesh2
  []
  # console = false
  [./console]
    type = Console
    output_linear = false
    output_nonlinear = false
  [../]
[]
