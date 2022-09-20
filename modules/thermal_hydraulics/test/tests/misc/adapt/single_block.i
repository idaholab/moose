[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

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
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 20
    A   = 1.0000000000e-04
    D_h  = 1.1283791671e-02
    f = 0.
    fp = eos
  []

  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe1:in'
    rho = 996.561962436227759
    vel = 1
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe1:out'
    p = 1e5
  []
[]

[Outputs]
  exodus = true
  show = 'rhoA rhouA rhoEA'

  [console]
    type = Console
    print_mesh_changed_info = true
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

  start_time = 0.0
  dt = 1e-5
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'

  [Adaptivity]
    initial_adaptivity = 0 # There seems to be a bug with non-zero initial adaptivity
    refine_fraction = 0.60
    coarsen_fraction = 0.30
    max_h_level = 4
  []
[]
