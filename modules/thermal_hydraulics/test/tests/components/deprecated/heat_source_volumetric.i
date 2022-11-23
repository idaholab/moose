[GlobalParams]
  scaling_factor_1phase = '1 1e-2 1e-4'
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
  [flow_channel]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10

    A = 1
    f = 0
    fp = fp
    closures = simple_closures

    initial_T = 310
    initial_p = 1e5
    initial_vel = 0
  []
  [wall1]
    type = SolidWall1Phase
    input = flow_channel:in
  []
  [wall2]
    type = SolidWall1Phase
    input = flow_channel:out
  []

  [heat_source]
    type = HeatSourceVolumetric
    flow_channel = flow_channel
    q = 1e3
  []
[]

[Postprocessors]
  [E_tot]
    type = ElementIntegralVariablePostprocessor
    variable = rhoEA
    execute_on = 'initial timestep_end'
  []

  [E_tot_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = E_tot
    execute_on = 'initial timestep_end'
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

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10

  start_time = 0.0
  dt = 0.1
  end_time = 1

  abort_on_solve_fail = true
[]
