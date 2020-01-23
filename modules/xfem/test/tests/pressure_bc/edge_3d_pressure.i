[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = false
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 9
  nz = 10
  xmin = -0.1
  xmax = 0.1
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
  elem_type = HEX8
[]

[UserObjects]
  [./square_planar_cut_uo]
    type = RectangleCutUserObject
    cut_data = '-0.2  0.0 -0.5
                -0.2  0.0  0.0
                 0.2  0.0  0.0
                 0.2  0.0 -0.5'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz'
  [../]
[]

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0 2.0 4.0 6.0 8.0'
    y = '0 1000 0 1000 0'
  [../]
[]

[DiracKernels]
  [./p_x]
    type = XFEMPressure
    variable = disp_x
    component = 0
    function = pressure
  [../]
  [./p_y]
    type = XFEMPressure
    variable = disp_y
    component = 1
    function = pressure
  [../]
  [./p_z]
    type = XFEMPressure
    variable = disp_z
    component = 2
    function = pressure
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    boundary = 'bottom top'
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 'bottom top'
    variable = disp_y
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    boundary = 'bottom top'
    variable = disp_z
    value = 0.0
  [../]
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
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 2.0
[]

[Outputs]
  file_base = edge_3d_pressure_out
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
