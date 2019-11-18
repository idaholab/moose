[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T_liquid = 483.613543446158
  initial_T_vapor  = 483.613543446158
  initial_p_liquid = 4e6
  initial_p_vapor  = 4e6
  initial_vel_liquid = 0
  initial_vel_vapor = 0
  initial_alpha_vapor = 0.5

  scaling_factor_2phase = '1e0
                           1e0 1e-3 1e-6
                           1e0 1e-3 1e-6'

  pressure_relaxation = true
  velocity_relaxation = true
  interface_transfer = true

  specific_interfacial_area_max_value = 10

  closures = simple

  spatial_discretization = cg
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasTwoPhaseFluidProperties
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel2Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1.
    D_h = 1.12837916709551
    f = 1.12837916709551
    f_interface = 0

    length = 1
    n_elems = 100
  [../]

  [./inlet]
    type = InletStagnationPressureTemperature2Phase
    input = 'pipe:in'

    p0_liquid = 4e6
    T0_liquid = 483.613543446158
    p0_vapor = 4e6
    T0_vapor = 483.613543446158
    alpha_vapor = 0.5
  [../]

  [./outlet]
    type = Outlet2Phase
    input = 'pipe:out'

    p_vapor = 3.95e6
    p_liquid = 3.95e6
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

  start_time = 0
  dt = 1e-5
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-2
  l_max_its = 100

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
