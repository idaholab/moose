# Tests that friction factor can be provided for 1-phase flow

f = 5

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 558
  initial_p = 7.0e6
  initial_vel = 0

  scaling_factor_1phase = '1e0 1e-2 1e-4'

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

[Functions]
  [f_func]
    type = ConstantFunction
    value = ${f}
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 1

    A   = 1.907720E-04
    D_h  = 1.698566E-02
    f = f_func

    fp = eos
  []

  [ht_pipe]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 559
    P_hf = 0.0489623493599167
    Hw = 50000
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'
    rho = 741.707129779398883
    vel = 2
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 7.0e6
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
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-2
  l_max_its = 30
[]

[Postprocessors]
  [f]
    type = ADElementIntegralMaterialProperty
    mat_prop = f_D
    block = pipe
  []
[]

[Outputs]
  csv = true
  show = 'f'
  execute_on = 'timestep_end'
[]
