# This is testing that controls are executed in the correct order
#
# If controls are executed in the right order, then T_inlet_ctrl
# reads the value of temperature (T = 345 K) from a function. Then
# this value is set into the BC and then is it sampled by a
# postprocessor whose value is then written into a CSV file.
#
# If controls were executed in the wrong order, we would sample the
# stagnation temperature function at time t = 0, which would give
# T = 360 K back, and we would see this value in the CSV file instead.

[GlobalParams]
  initial_p = 100.e3
  initial_vel = 1.0
  initial_T = 350.
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
  [pipe1]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '1 0 0'
    length = 15.0
    n_elems = 10
    A    = 0.01
    D_h  = 0.1
    f = 0.01
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = 100.e3
    T0 = 355.
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 100.0e3
  []
[]

[Functions]
  # Stagnation temperature in time
  [T0_fn]
    type = PiecewiseLinear
    x = '0   1e-5'
    y = '360 345'
  []
[]

[ControlLogic]
  [set_inlet_value_ctrl]
    type = SetComponentRealValueControl
    component = inlet
    parameter = T0
    value = T_inlet_ctrl:value
  []

  [T_inlet_ctrl]
    type = GetFunctionValueControl
    function = T0_fn
  []
[]

[Postprocessors]
  [T_ctrl]
    type = RealComponentParameterValuePostprocessor
    component = inlet
    parameter = T0
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

  start_time = 0
  dt = 1e-5
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 5
[]

[Outputs]
  csv = true
[]
