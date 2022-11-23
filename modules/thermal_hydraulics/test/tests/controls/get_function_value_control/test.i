# This is testing that the values obtained by GetFunctionValueControl are used.
# Function T0_fn prescribes values for T_inlet_fn control. We output the function
# values via a postprocessor `T_fn` and the control data values via another
# postprocessor `T_ctrl`. Those two values have to be equal.

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
    T0 = 350.
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 100.0e3
  []
[]

[Functions]
  [T0_fn]
    type = PiecewiseLinear
    x = '0 1'
    y = '350 345'
  []
[]

[ControlLogic]
  [T_inlet_fn]
    type = GetFunctionValueControl
    function = T0_fn
  []
[]

[Postprocessors]
  [T_fn]
    type = FunctionValuePostprocessor
    function = T0_fn
  []

  [T_ctrl]
    type = RealControlDataValuePostprocessor
    control_data_name = T_inlet_fn:value
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

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 5

  start_time = 0.0
  end_time = 1
[]

[Outputs]
  csv = true
[]
