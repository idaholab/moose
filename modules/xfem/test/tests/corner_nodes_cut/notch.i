[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '-0.26 0.0 0.0 0.1'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
[]

[Mesh]
  file = notch.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    strain = SMALL
  [../]
[]

[BCs]
  [./top_x]
    type = DirichletBC
    boundary = 102
    variable = disp_x
    value = 0.0
  [../]
  [./top_y]
    type = DirichletBC
    boundary = 102
    variable = disp_y
    value = 0.1
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 101
    variable = disp_y
    value = -0.1
  [../]
  [./bottom_x]
    type = DirichletBC
    boundary = 101
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
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-9

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
