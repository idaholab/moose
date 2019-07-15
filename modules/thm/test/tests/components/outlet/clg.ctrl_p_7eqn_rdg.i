[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p_liquid = 1e5
  initial_p_vapor = 1.01e5
  initial_T_liquid = 372
  initial_T_vapor = 375
  initial_vel_liquid = 0.0
  initial_vel_vapor = 0.0
  initial_alpha_vapor = 0.01

  scaling_factor_2phase = '1 1 1 1e-5 1 1 1e-6'

  closures = simple
  specific_interfacial_area_min_value = 1e-15
  specific_interfacial_area_max_value = 1000

  explicit_alpha_gradient = true
  explicit_acoustic_impedance = true

  wall_mass_transfer = false
  interface_transfer = false

  spatial_discretization = rDG
  rdg_slope_reconstruction = none
[]

[FluidProperties]
  [./fp]
    type = StiffenedGas7EqnFluidProperties
    emit_on_nan = warning
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel2Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 25

    A   = 1.0000000000e-04
    D_h = 1.1283791671e-02

    f = 0.0
    f_interface = 0

    fp = fp
  [../]

  [./inlet]
    type = InletStagnationPressureTemperature
    input = 'pipe:in'
    p0_liquid = 1.001e5
    p0_vapor = 1.001e5
    T0_liquid = 372
    T0_vapor = 375
    alpha_vapor = 0.01
  [../]

  [./outlet]
    type = Outlet
    input = 'pipe:out'
    p_liquid = 1e5
    p_vapor = 1e5
  [../]
[]

[Functions]
  [./outlet_p_liquid_fn]
    type = PiecewiseLinear
    x = '0   1'
    y = '1e5 1.001e5'
  [../]
  [./outlet_p_vapor_fn]
    type = PiecewiseLinear
    x = '0   1'
    y = '1.01e5 1.011e5'
  [../]
[]

[Controls]
  [./set_outlet_p_liquid]
    type = TimeFunctionControl
    component = outlet
    parameter = p_liquid
    function = outlet_p_liquid_fn
  [../]
  [./set_outlet_p_vapor]
    type = TimeFunctionControl
    component = outlet
    parameter = p_vapor
    function = outlet_p_vapor_fn
  [../]
[]

[Postprocessors]
  [./outlet_p_liquid]
    type = RealComponentParameterValuePostprocessor
    component = outlet
    parameter = p_liquid
  [../]
  [./outlet_p_vapor]
    type = RealComponentParameterValuePostprocessor
    component = outlet
    parameter = p_vapor
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

  start_time = 0.0
  dt = 0.25
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  csv = true
[]
