[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple

  spatial_discretization = rDG
  rdg_slope_reconstruction = none

  scaling_factor_2phase = '1 1e-3 1e-3 1e-8 1 1 1e-5'
[]

[FluidProperties]
  [./fp]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel2Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 50

    A = 0.5

    f = 0.0
    f_interface = 0.0

    fp = fp

    initial_alpha_vapor = 0.5
    initial_p_liquid = 1e5
    initial_p_vapor  = 1e5
    initial_T_liquid = 349.144404977276
    initial_T_vapor  = 349.144404977276
    initial_vel_liquid = 1.0
    initial_vel_vapor  = 1.0

    pressure_relaxation = false
    velocity_relaxation = false
    interface_transfer  = false
    wall_mass_transfer  = false
  [../]

  [./inlet]
    type = InletDensityVelocity2Phase
    input = 'pipe:in'
    alpha_vapor = 0.5
    rho_liquid = 0
    rho_vapor  = 5
    vel_liquid = 0.5
    vel_vapor  = 1.5
  [../]

  [./outlet]
    type = Outlet2Phase
    input = 'pipe:out'
    p_liquid = 1e5
    p_vapor  = 1e5
  [../]
[]

[Functions]
  [./inlet_alpha_vapor_fn]
    type = PiecewiseLinear
    x = '0 0.2'
    y = '0.5 0.6'
  [../]
  [./inlet_rho_liquid_fn]
    type = PiecewiseLinear
    x = '0 0.2'
    y = '1170 1175'
  [../]
  [./inlet_rho_vapor_fn]
    type = PiecewiseLinear
    x = '0 0.2'
    y = '0.5 1.5'
  [../]
  [./inlet_vel_liquid_fn]
    type = PiecewiseLinear
    x = '0 0.2'
    y = '0 0.5'
  [../]
  [./inlet_vel_vapor_fn]
    type = PiecewiseLinear
    x = '0 0.2'
    y = '0 1.0'
  [../]
[]

[Controls]
  [./set_inlet_value_alpha_vapor]
    type = TimeFunctionControl
    component = inlet
    parameter = alpha_vapor
    function = inlet_alpha_vapor_fn
  [../]
  [./set_inlet_value_rho_liquid]
    type = TimeFunctionControl
    component = inlet
    parameter = rho_liquid
    function = inlet_rho_liquid_fn
  [../]
  [./set_inlet_value_rho_vapor]
    type = TimeFunctionControl
    component = inlet
    parameter = rho_vapor
    function = inlet_rho_vapor_fn
  [../]
  [./set_inlet_value_vel_liquid]
    type = TimeFunctionControl
    component = inlet
    parameter = vel_liquid
    function = inlet_vel_liquid_fn
  [../]
  [./set_inlet_value_vel_vapor]
    type = TimeFunctionControl
    component = inlet
    parameter = vel_vapor
    function = inlet_vel_vapor_fn
  [../]
[]

[Postprocessors]
  [./inlet_alpha_vapor]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = alpha_vapor
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./inlet_rho_liquid]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = rho_liquid
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./inlet_rho_vapor]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = rho_vapor
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./inlet_vel_liquid]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = vel_liquid
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./inlet_vel_vapor]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = vel_vapor
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  dt = 0.1
  num_steps = 2
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  [./out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  [../]
[]
