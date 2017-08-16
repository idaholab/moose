
[Mesh]
  file = incremental_slip.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[] # Variables

[Functions]
  [./slave_x]
    type = PiecewiseLinear
    x = '0 1 2   3 4 5  6    7 8   9'
    y = '0 0 0.5 0 0 0 -0.25 0 0.5 0'
  [../]
  [./slave_y]
    type = PiecewiseLinear
    x = '0  1     9'
    y = '0 -0.15 -0.15'
  [../]
  [./slave_z]
    type = PiecewiseLinear
    x = '0 1  2   3 4 5 6    7  8   9'
    y = '0 0 -0.5 0 0 0 0.25 0 -0.5 0'
  [../]

  [./master_x]
    type = PiecewiseLinear
    x = '0 1  2 3 4   5 6    7 8   9'
    y = '0 0  0 0 0.5 0 0.25 0 0.5 0'
  [../]
  [./master_y]
    type = PiecewiseLinear
    x = '0 9'
    y = '0 0'
  [../]
  [./master_z]
    type = PiecewiseLinear
    x = '0 1  2 3 4   5  6    7  8   9'
    y = '0 0  0 0 0.5 0 -0.25 0 -0.5 0'
  [../]
[]

[AuxVariables]

  [./inc_slip_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./inc_slip_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./inc_slip_z]
    order = FIRST
    family = LAGRANGE
  [../]

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[AuxKernels]
  [./inc_slip_x]
    type = PenetrationAux
    variable = inc_slip_x
    quantity = incremental_slip_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./inc_slip_y]
    type = PenetrationAux
    variable = inc_slip_y
    quantity = incremental_slip_y
    boundary = 3
    paired_boundary = 2
  [../]
  [./inc_slip_z]
    type = PenetrationAux
    variable = inc_slip_z
    quantity = incremental_slip_z
    boundary = 3
    paired_boundary = 2
  [../]
[]

[Contact]
  [./dummy_name]
    master = 2
    slave = 3
    penalty = 1e7
  [../]
[]

[BCs]

  [./slave_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = slave_x
  [../]

  [./slave_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = slave_y
  [../]

  [./slave_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 4
    function = slave_z
  [../]


  [./master_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '1 2'
    function = master_x
  [../]

  [./master_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '1 2'
    function = master_y
  [../]

  [./master_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = '1 2'
    function = master_z
  [../]

[] # BCs

[Materials]

  [./stiffStuff1]
    type = Elastic
    block = 1
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff2]
    type = Elastic
    block = 2
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
[] # Materials

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu            superlu_dist'


  line_search = 'none'


  nl_abs_tol = 1e-8

  l_max_its = 100
  nl_max_its = 10
  dt = 1.0
  num_steps = 9
[] # Executioner

[Outputs]
  file_base = incremental_slip_out
  exodus = true
[] # Outputs
