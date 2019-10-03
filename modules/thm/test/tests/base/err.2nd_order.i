[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e6
  initial_T = 353.1
  initial_vel = 0.0

  2nd_order_mesh = true

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 100

    A = 1.0e-4
    D_h = 1.128379e-2

    f = 0.0

    fp = eos
  [../]

  [./inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'
    vel = 1.0
    rho = 972.2240858943135
  [../]

  [./outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1.0e6
    legacy = true
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  dt = 0.1
  dtmin = 1.e-7

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  start_time = 0.0
  end_time = 4.0

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]
