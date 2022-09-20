# This test tests conservation of energy at steady state for 1-phase flow when a
# heat flux is specified. Conservation is checked by comparing the integral of
# the heat flux against the difference of the boundary fluxes.

[GlobalParams]
  initial_p = 7.0e6
  initial_vel = 0
  initial_T = 513

  gravity_vector = '0.0 0.0 0.0'

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
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '0 0 1'
    length = 3.66
    n_elems = 10

    A = 1.907720E-04
    D_h = 1.698566E-02
    f = 0.0

    fp = eos
  []

  [ht_pipe]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = pipe
    q_wall = 1.0e5
    Hw = 1.0e4
    P_hf = 4.4925e-2
  []

  [inlet]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [outlet]
    type = SolidWall1Phase
    input = 'pipe:out'
  []
[]

[Postprocessors]
  [E]
    type = ElementIntegralVariablePostprocessor
    variable = rhoEA
    execute_on = 'initial timestep_end'
  []

  [E_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E
    execute_on = 'initial timestep_end'
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
  abort_on_solve_fail = true
  dt = 1

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-7
  nl_max_its = 50

  l_tol = 1e-3
  l_max_its = 60

  start_time = 0
  num_steps = 10
[]

[Outputs]
  [out]
    type = CSV
    show = 'E_change'
  []
  [console]
    type = Console
    show = 'E_change'
  []
[]
