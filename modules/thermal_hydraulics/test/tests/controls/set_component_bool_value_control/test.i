# This is testing that the values set by SetComponentBoolValueControl are used.
# The `trip_ctrl` component produces a boolean value that is set in the
# `turbine` component to switch it on/off.

[GlobalParams]
  initial_p = 100.e3
  initial_vel = 1.0
  initial_T = 350.
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  scaling_factor_1phase = '1 1e-2 1e-4'
  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'fch1:in'
    p0 = 100.e3
    T0 = 350.
  []

  [fch1]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 10
    A    = 0.01
    D_h  = 0.1
    f = 0.01
  []

  [turbine]
    type = SimpleTurbine1Phase
    position = '1 0 0'
    connections = 'fch1:out fch2:in'
    volume = 1
    on = false
    power = 1
    use_scalar_variables = false
  []

  [fch2]
    type = FlowChannel1Phase
    fp = fp
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 10
    A    = 0.01
    D_h  = 0.1
    f = 0.01
  []

  [outlet]
    type = Outlet1Phase
    input = 'fch2:out'
    p = 100.0e3
  []
[]

[Functions]
  [trip_fn]
    type = PiecewiseLinear
    xy_data = '
      0 1
      1 2'
  []
[]

[ControlLogic]
  [trip_ctrl]
    type = UnitTripControl
    condition = 'val > 1.5'
    symbol_names = 'val'
    symbol_values = 'trip_fn'
  []

  [set_comp_value]
    type = SetComponentBoolValueControl
    component = turbine
    parameter = on
    value = trip_ctrl:state
  []
[]

[Postprocessors]
  [on_ctrl]
    type = BoolComponentParameterValuePostprocessor
    component = turbine
    parameter = on
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  abort_on_solve_fail = true

  solve_type = NEWTON
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 5

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  start_time = 0.0
  end_time = 1
[]

[Outputs]
  [out]
    type = CSV
    show = 'on_ctrl'
  []
[]
