[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e6
  initial_T = 300
  initial_vel = 0

  scaling_factor_1phase = '1 1 1e-5'

  closures = simple
[]

[FluidProperties]
  [eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    fp = eos

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10

    A = 1

    f = 0.
  []

  [inlet]
    type = SolidWall1Phase
    input = 'pipe1:in'
  []
  [outlet]
    type = SolidWall1Phase
    input = 'pipe1:out'
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
  scheme = bdf2

  start_time = 0.0
  dt = 0.01
  num_steps = 2
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10
[]

[Postprocessors]
  [cfl]
    type = ADCFLTimeStepSize
    CFL = 0.5
    vel_names = 'vel'
    c_names = 'c'
  []
[]

[Outputs]
  csv = true
[]
