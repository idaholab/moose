# This input file tests whether we can converge to the semi-analytical
# solution for flow in a 2D wedge.
[GlobalParams]
  gravity = '0 0 0'
  rho = 1
  mu = 1
[]

[Mesh]
  # file = wedge_4x6.e
  file = wedge_8x12.e
  # file = wedge_16x24.e
  # file = wedge_32x48.e
  # file = wedge_64x96.e
[]

[MeshModifiers]
  [./corner_node]
    # I found that pinning the pressure to zero on any part of the
    # boundary, i.e.
    # coord = (1, 0) # centerline left
    # coord = (2, 0) # centerline right
    # coord = (1.4488887394336025, 0.3882285676537811) # halfway along the top solid wall
    # didn't make much difference in the final solution.
    type = AddExtraNodeset
    new_boundary = pinned_node
    coord = '1.4488887394336025, 0.3882285676537811'
  [../]
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
    p = p
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
    p = p
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
    p = p
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
    boundary = 'inlet outlet'
    function = 'vel_x_exact'
  [../]
  [./vel_y_inlet]
    type = FunctionDirichletBC
    variable = vel_y
    boundary = 'inlet outlet'
    function = 'vel_y_exact'
  [../]
  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
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
  nl_abs_tol = 1e-12
  nl_max_its = 10
  l_tol = 1e-6
  l_max_its = 300
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
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
    alpha_degrees = 15 # Must match mesh and PiecewiseLinear function!
    Re = 30 # Must match PiecewiseLinear function!
    component = 0
    f = f_theta
  [../]
  [./vel_y_exact]
    type = WedgeFunction
    alpha_degrees = 15 # Note: must match mesh and PiecewiseLinear function!
    Re = 30 # Must match PiecewiseLinear function!
    component = 1
    f = f_theta
  [../]
[]

[Postprocessors]
  [./vel_x_L2_error]
    type = ElementL2Error
    variable = vel_x
    function = vel_x_exact
    execute_on = 'initial timestep_end'
  [../]
  [./vel_y_L2_error]
    type = ElementL2Error
    variable = vel_y
    function = vel_y_exact
    execute_on = 'initial timestep_end'
  [../]
[]
