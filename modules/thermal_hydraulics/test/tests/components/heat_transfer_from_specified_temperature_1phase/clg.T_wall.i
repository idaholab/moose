[GlobalParams]
  initial_p = 0.1e6
  initial_vel = 0
  initial_T = 300
  scaling_factor_1phase = '1e+0 1e-2 1e-4'
  closures = simple_closures
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
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
    fp = eos
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 50

    A = 3.14e-2
    f = 0.1
  []

  [ht_pipe1]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe1
    T_wall = 300
    Hw = 0
  []

  [inlet1]
    type = InletDensityVelocity1Phase
    input = 'pipe1:in'
    rho = 996.557482499661660
    vel = 1
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 0.1e6
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
  scheme = 'bdf2'

  start_time = 0
  dt = 0.05
  num_steps = 20
  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 30
[]

[Outputs]
  csv = true
[]

[Functions]
  [T_wall_fn]
    type = PiecewiseLinear
    x = '0 1'
    y = '310 320'
  []
[]

[ControlLogic]
  [pipe_T_wall_ctrl]
    type = TimeFunctionComponentControl
    component = ht_pipe1
    parameter = T_wall
    function = T_wall_fn
  []
[]

[Postprocessors]
  [T_wall]
    type = RealComponentParameterValuePostprocessor
    component = ht_pipe1
    parameter = T_wall
  []
[]
