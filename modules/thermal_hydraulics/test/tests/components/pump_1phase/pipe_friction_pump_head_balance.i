# This test balances the pipe friction pressure drop with the pump head pressure rise and runs to steady state.

[GlobalParams]
  initial_T = 393.15
  initial_vel = 0.0
  A = 0.567
  fp = fp
  scaling_factor_1phase = '0.04 0.04 0.04e-5'
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
    position = '0 0 0'
    orientation = '1 0 0'
    length = 10
    initial_p = 1.35e+07
    n_elems = 20
    f = 5000
    gravity_vector = '0 0 0'
  []
  [pump]
    type = Pump1Phase
    connections = 'pipe1:out pipe1:in'
    position = '1.02 0 0'
    initial_p = 1.3e+07
    initial_vel_x = 1
    initial_vel_y = 0
    initial_vel_z = 0
    scaling_factor_rhoV  = 1
    scaling_factor_rhouV = 1
    scaling_factor_rhoEV = 1e-5
    head = 8
    volume = 0.567
    A_ref = 0.567
    use_scalar_variables = false
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  start_time = 0
  dt = 1.e-3
  num_steps = 38
  abort_on_solve_fail = true
  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15
  l_tol = 1e-4
  l_max_its = 10
  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]


[Outputs]
  [out_x]
    type = Exodus
    show = 'p T vel'
  []
  velocity_as_vector = false
[]
