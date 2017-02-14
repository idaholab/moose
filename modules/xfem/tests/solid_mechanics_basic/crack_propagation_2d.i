[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[XFEM]
  #             x0   y0   x1   y1   t0    t1
  cut_data = ' 1.0  0.5  0.7  0.5  0.0   0.0'
  qrule = volfrac
  output_cut_plane = true
  use_crack_growth_increment = true
  crack_growth_increment = 0.2
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 11
  ny = 11
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD4
  displacements = 'disp_x disp_y'
[]

[UserObjects]
  [./xfem_marker_uo]
    type = XFEMMaterialTensorMarkerUserObject
    execute_on = timestep_end
    tensor = stress
    quantity = MaxPrincipal
    threshold = 5e+1
    average = true
  [../]
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
  [../]
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x='0  50   100'
    y='0  0.02 0.1'
  [../]
[]

[BCs]
  [./bottomx]
    type = PresetBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./topx]
    type = PresetBC
    boundary = top
    variable = disp_x
    value = 0.0
  [../]
  [./topy]
    type = FunctionPresetBC
    boundary = top
    variable = disp_y
    function = pull
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
    thermal_expansion = 0.02
    t_ref = 0.5
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
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-12

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 2.0
  num_steps = 5000

  max_xfem_update = 1
[]

[Outputs]
  file_base = crack_propagation_2d_out
  exodus = true
  execute_on = timestep_end
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
