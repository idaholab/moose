# This is a part of T_wall_action test. See the master file for details.

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

[AuxVariables]
  [Hw]
    family = monomial
    order = constant
    block = pipe1
  []
[]

[AuxKernels]
  [Hw_ak]
    type = ADMaterialRealAux
    variable = Hw
    property = 'Hw'
  []
[]

[UserObjects]
  [T_uo]
    type = LayeredAverage
    direction = y
    variable = T
    num_layers = 10
    block = pipe1
  []
  [Hw_uo]
    type = LayeredAverage
    direction = y
    variable = Hw
    num_layers = 10
    block = pipe1
  []
[]


[Components]
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 1 0'
    length = 1
    n_elems = 10

    A   = 1.28584e-01
    D_h = 8.18592e-01
    f = 0.01

    fp = eos
  []

  [hxconn]
    type = HeatTransferFromExternalAppTemperature1Phase
    flow_channel = pipe1
    Hw = 10000
    P_hf = 6.28319e-01
    initial_T_wall = 300.
    var_type = elemental
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = 10
    T = 400
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 1e5
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [T_wall_avg]
    type = ElementAverageValue
    variable = T_wall
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [htc_avg]
    type = ElementAverageValue
    variable = Hw
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [T_avg]
    type = ElementAverageValue
    variable = T
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  dt = 0.1
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

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
[]

[Outputs]
  [out]
    type = Exodus
    show = 'T_wall T Hw'
  []
[]
