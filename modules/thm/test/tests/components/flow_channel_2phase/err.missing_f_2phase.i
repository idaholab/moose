[GlobalParams]
  gravity_vector = '0 0 0'

  initial_vel_liquid = 0
  initial_vel_vapor = 0
  initial_p_liquid = 1e5
  initial_p_vapor = 1e5
  initial_T_liquid = 300
  initial_T_vapor = 300
  initial_alpha_vapor = 0.01

  closures = simple

  spatial_discretization = RDG
  rdg_slope_reconstruction = none
[]

[FluidProperties]
  [./water]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel2Phase
    fp = water
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1.
    length = 1
    n_elems = 10
  [../]

  [./inlet]
    type = InletStagnationPressureTemperature2Phase
    input = 'pipe:in'
    p0_liquid = 10
    T0_liquid = 10
    p0_vapor = 10
    T0_vapor = 10
    alpha_vapor = 0.01
  [../]

  [./outlet]
    type = Outlet2Phase
    input = 'pipe:out'
    p_liquid = 10
    p_vapor = 10
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  dt = 1e-4
  dtmin = 1.e-7

  solve_type = 'PJFNK'
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-8
  l_max_its = 100

  start_time = 0.0
  num_steps = 10

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  exodus = true
[]
