[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[XFEM]
  cut_type = 'line_segment_2d'
  cut_data = '-0.001   0.5   0.3  0.5   0.0 1.0
               0.3       0.5   0.7  0.5   1.0 2.0'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 9
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD9
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = false
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
    type = FunctionPresetBC
    boundary = 1
    variable = disp_x
    function = right_disp_x
  [../]
  [./top_y]
    type = FunctionPresetBC
    boundary = 2
    variable = disp_y
    function = top_disp_y
  [../]
  [./bottom_y]
    type = PresetBC
    boundary = 0
    variable = disp_y
    value = 0.0
  [../]
  [./left_x]
    type = PresetBC
    boundary = 3
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./linelast]
    type = LinearIsotropicMaterial
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    poissons_ratio = 0.3
    youngs_modulus = 1e6
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
  end_time = 2.0
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
