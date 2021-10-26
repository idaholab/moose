# This input file solves the Jeffery-Hamel problem with the exact
# solution's outlet BC replaced by a natural BC.  This problem does
# not converge to the analytical solution, although the flow at the
# outlet still "looks" reasonable.
[GlobalParams]
  gravity = '0 0 0'

  # Params used by the WedgeFunction for computing the exact solution.
  # The value of K is only required for comparing the pressure to the
  # exact solution, and is computed by the associated jeffery_hamel.py
  # script.
  alpha_degrees = 15
  Re = 30
  K = -9.78221333616
  f = f_theta
[]

[Mesh]
  file = wedge_8x12.e
[]

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]
  [./p]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  [../]
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  [../]
[]

[BCs]
  [./vel_x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'top_wall bottom_wall'
    value = 0.0
  [../]
  [./vel_y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'top_wall bottom_wall'
    value = 0.0
  [../]
  [./vel_x_inlet]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'inlet'
    function = 'vel_x_exact'
  [../]
  [./vel_y_inlet]
    type = FunctionDirichletBC
    variable = vel_y
    boundary = 'inlet'
    function = 'vel_y_exact'
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
[]

[Preconditioning]
  [./SMP_NEWTON]
    type = SMP
    full = true
    solve_type = NEWTON
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-2
  num_steps = 5
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = '300                bjacobi  ilu          4'
  line_search = none
  nl_rel_tol = 1e-13
  nl_abs_tol = 1e-11
  nl_max_its = 10
  l_tol = 1e-6
  l_max_its = 300
[]

[Outputs]
  exodus = true
[]

[Functions]
  [./f_theta]
    # Non-dimensional solution values f(eta), 0 <= eta <= 1 for
    # alpha=15deg, Re=30.  Note: this introduces an input file
    # ordering dependency: this Function must appear *before* the two
    # function below which use it since apparently proper dependency
    # resolution is not done in this scenario.
    type = PiecewiseLinear
    data_file = 'f.csv'
    format = 'columns'
  [../]
  [./vel_x_exact]
    type = WedgeFunction
    var_num = 0
    mu = 1
    rho = 1
  [../]
  [./vel_y_exact]
    type = WedgeFunction
    var_num = 1
    mu = 1
    rho = 1
  [../]
[]
