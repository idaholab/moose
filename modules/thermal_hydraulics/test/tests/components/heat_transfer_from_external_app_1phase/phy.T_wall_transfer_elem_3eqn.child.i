# This is a part of phy.T_wall_transfer_elem_3eqn test. See the master file for details.

[GlobalParams]
  initial_p = 1.e5
  initial_vel = 0.
  initial_T = 300.

  closures = simple_closures
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
    length = 1
    n_elems = 10

    A   = 9.6858407346e-01
    D_h = 6.1661977237e+00
    f = 0.01

    fp = eos
  []

  [hxconn]
    type = HeatTransferFromExternalAppTemperature1Phase
    flow_channel = pipe1
    Hw = 3000
    P_hf = 6.2831853072e-01
    initial_T_wall = 300.
    var_type = elemental
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = 1
    T = 300
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 1e5
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
  dt = 0.5
  dtmin = 1e-7
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-4
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 300

  start_time = 0.0
  end_time = 5
[]

[Outputs]
  [out]
    type = Exodus
    show = 'T_wall'
  []
[]
