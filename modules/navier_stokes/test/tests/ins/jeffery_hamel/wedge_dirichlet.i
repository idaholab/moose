# This input file tests whether we can converge to the semi-analytical
# solution for flow in a 2D wedge.
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
  [file]
    type = FileMeshGenerator
    # file = wedge_4x6.e
    file = wedge_8x12.e
    # file = wedge_16x24.e
    # file = wedge_32x48.e
    # file = wedge_64x96.e
  []
  [./corner_node]
    # Pin is on the centerline of the channel on the left-hand side of
    # the domain at r=1.  If you change the domain, you will need to
    # update this pin location for the pressure exact solution to
    # work.
    type = ExtraNodesetGenerator
    new_boundary = pinned_node
    coord = '1 0'
    input = file
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

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'rho mu'
    prop_values = '1  1'
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
    # alpha=15 deg, Re=30.  Note: this introduces an input file
    # ordering dependency: this Function must appear *before* the two
    # functions below which use it since apparently proper dependency
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
  [./p_exact]
    type = WedgeFunction
    var_num = 2
    mu = 1
    rho = 1
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
  [./p_L2_error]
    type = ElementL2Error
    variable = p
    function = p_exact
    execute_on = 'initial timestep_end'
  [../]
[]
