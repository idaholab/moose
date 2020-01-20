[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[Mesh]
  file = quarter_sym.e
  displacements = 'disp_x disp_y disp_z'
[]

[UserObjects]
  [./ellip_cut_uo]
    type = EllipseCutUserObject
    cut_data = '-0.5 -0.5 0
                -0.5 -0.1 0
                 0.1 -0.5 0'
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
  [./vonmises_stress]
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
  [./vonmises_stress]
    type = MaterialTensorAux
    tensor = stress
    variable = vonmises_stress
    quantity = vonmises
    execute_on = timestep_end
  [../]
[]

[Functions]
  [./top_trac_z]
    type = ConstantFunction
    value = 10
  [../]
[]


[BCs]
  [./top_z]
    type = FunctionNeumannBC
    boundary = 2
    variable = disp_z
    function = top_trac_z
  [../]
  [./bottom_x]
    type = DirichletBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 1
    variable = disp_y
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    boundary = 1
    variable = disp_z
    value = 0.0
  [../]
  [./sym_y]
    type = DirichletBC
    boundary = 3
    variable = disp_y
    value = 0.0
  [../]
  [./sym_x]
    type = DirichletBC
    boundary = 4
    variable = disp_x
    value = 0.0
  [../]
[]

[Materials]
  [./linelast]
    type = Elastic
    block = 1
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
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  file_base = elliptical_crack_out
  exodus = true
  execute_on = timestep_end
  [./console]
    type = Console
    output_linear = true
  [../]
[]
