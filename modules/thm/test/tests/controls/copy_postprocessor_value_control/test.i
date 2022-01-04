# This is testing that the values copied by CopyPostprocessorValueControl are used.
# A postprocessor T_pt samples value at point (0, 0, 0), those values are then
# read in by CopyPostprocessorValueControl and then we output this value. The values
# are lagged by one time step, because controls are executed at the beginning
# of the time step and postprocessors at the end of the time step. Note that
# CopyPostprocessorValueControl is added when a postprocessor is created. That's why
# you do not see the object in this input file.

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
    T0 = 340.
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 100.0e3
  []
[]

[Postprocessors]
  [T_pt]
    type = SideAverageValue
    boundary = pipe1:in
    variable = T
    execute_on = 'initial timestep_end'
  []

  [T_ctrl]
    type = RealControlDataValuePostprocessor
    control_data_name = T_pt
    execute_on = timestep_end
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
  num_steps = 3
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
