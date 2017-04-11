[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[XFEM]
  cut_type = 'square_cut_3d' # rectangular cut plane
  cut_data = ' -0.001 0.5 -0.001
                0.401 0.5 -0.001
                0.401 0.5  0.201
               -0.001 0.5  0.201'
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 2
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.2
  elem_type = HEX27
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  []

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
    execute_on = timestep_end
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
    execute_on = timestep_end
  [../]
[]

[Functions]
  [./top_trac_y]
    type = ConstantFunction
    value = 10
  [../]
[]

[BCs]
  [./top_y]
    type = FunctionNeumannBC
    boundary = top
    variable = disp_y
    function = top_trac_y
  [../]
  [./bottom_x]
    type = PresetBC
    boundary = bottom
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = PresetBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
  [./bottom_z]
    type = PresetBC
    boundary = bottom
    variable = disp_z
    value = 0.0
  [../]
[]

[Materials]
  [./linelast]
    type = Elastic
    block = 0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    poissons_ratio = 0.3
    youngs_modulus = 207000
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
  end_time = 1.0
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
[]
