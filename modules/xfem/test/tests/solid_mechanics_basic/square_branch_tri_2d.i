[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  elem_type = TRI3
[]

[UserObjects]
  [./line_seg_cut_uo0]
    type = LineSegmentCutUserObject
    cut_data = '-1.0000e-10   6.6340e-01   6.6340e-01  -1.0000e-10'
    time_start_cut = 0.0
    time_end_cut = 1.0
  [../]
  [./line_seg_cut_uo1]
    type = LineSegmentCutUserObject
    cut_data = '3.3120e-01   3.3200e-01   1.0001e+00   3.3200e-01'
    time_start_cut = 1.0
    time_end_cut = 2.0
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    planar_formulation = PLANE_STRAIN
    add_variables = true
  [../]
[]

[Functions]
  [./right_disp_x]
    type = PiecewiseLinear
    x = '0  1.0    2.0   3.0'
    y = '0  0.005  0.01  0.01'
  [../]
  [./top_disp_y]
    type = PiecewiseLinear
    x = '0  1.0    2.0   3.0'
    y = '0  0.005  0.01  0.01'
  [../]
[]

[BCs]
  [./right_x]
    type = FunctionDirichletBC
    boundary = 1
    variable = disp_x
    function = right_disp_x
  [../]
  [./top_y]
    type = FunctionDirichletBC
    boundary = 2
    variable = disp_y
    function = top_disp_y
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 0
    variable = disp_y
    value = 0.0
  [../]
  [./left_x]
    type = DirichletBC
    boundary = 3
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeLinearElasticStress
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
  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 2.2
  num_steps = 5000
[]

[Outputs]
  file_base = square_branch_tri_2d_out
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
