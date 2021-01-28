[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 0.2e6
  initial_vel = 0
  initial_T = 300

  scaling_factor_1phase = '1. 1. 1.'

  closures = simple

  spatial_discretization = cg
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel1Phase
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 100

    A = 1.907720E-04
    f = 0.0

    fp = eos
  [../]

  [./inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'

    rho = 996.601
    vel = 1
  [../]

  [./outlet]
    type = Outlet1Phase
    input = 'pipe:out'

    p = 0.2e6
    legacy = true
  [../]
[]

[Functions]
  [./outlet_p_fn]
    type = PiecewiseLinear
    x = '0 1'
    y = ' 0.2e6 0.1e6'
  [../]
[]

[ControlLogic]
  [./outlet_p_ctrl]
    type = TimeFunctionComponentControl
    component = outlet
    parameter = p
    function = outlet_p_fn
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

  start_time = 0
  dt = 1e-1
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  [./Quadrature]
    type = TRAP
    order = FIRST
  [../]
[]

[Postprocessors]
  [./p_outlet]
    type = RealComponentParameterValuePostprocessor
    component = outlet
    parameter = p
  [../]
[]

[Outputs]
  [./out]
    type = CSV
  [../]
[]
