# Tests that an error is thrown if the user specifies a mixture of heat source
# types (temperature and heat flux).

[GlobalParams]
  initial_T = 300
  initial_p = 100e3
  initial_vel = 0

  closures = simple_closures
[]

[FluidProperties]
  [fp_water]
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
  [pipe]
    type = FlowChannel1Phase
    fp = fp_water
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1
    f = 0
    length = 1
    n_elems = 1
  []

  [ht1]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = pipe
    q_wall = 1
    P_hf = 1
    Hw = 1
  []

  [ht2]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 300
    P_hf = 1
    Hw = 1
  []

  [left]
    type = SolidWall
    input = 'pipe:in'
  []

  [right]
    type = SolidWall
    input = 'pipe:out'
  []
[]

[Preconditioning]
  [preconditioner]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       mumps'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1
  num_steps = 1

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 5

  l_tol = 1e-10
  l_max_its = 10

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]
