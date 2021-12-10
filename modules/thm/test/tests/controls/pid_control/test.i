# This test "measures" the liquid temperature at location (10, 0, 0) on a 15 meters
# long pipe and adjusts the inlet stagnation temperature using a PID controller with
# set point at 340 K.  The pipe is filled with water at T = 350 K. The purpose is to
# make sure that the channel fills with colder liquid and levels at the set point
# value. In steady state there should be a flat temperature profile at ~340 K.

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
    A   = 0.01
    D_h = 0.1
    f = 0.01
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = 105.e3
    T0 = 300.
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 100.0e3
  []
[]

[ControlLogic]
  [T_set_point]
    type = GetFunctionValueControl
    function = 340
  []

  [pid_ctrl]
    type = PIDControl
    input = T_reading
    set_point = T_set_point:value
    K_i = 0.05
    K_p = 0.2
    K_d = 0.1
    initial_value = 340
  []

  [set_inlet_value]
    type = SetComponentRealValueControl
    component = inlet
    parameter = T0
    value = pid_ctrl:output
  []
[]

[Postprocessors]
  [T_reading]
    type = PointValue
    point = '10 0 0'
    variable = T
    execute_on = timestep_begin
  []

  [T_inlet]
    type = PointValue
    point = '0 0 0'
    variable = T
    execute_on = timestep_begin
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
  dt = 5
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 5

  start_time = 0.0
  end_time = 300.0
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'final'
  []

  [console]
    type = Console
    max_rows = 1
  []
  print_linear_residuals = false
[]
