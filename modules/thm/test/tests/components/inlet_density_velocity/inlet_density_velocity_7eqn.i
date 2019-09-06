[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple

  spatial_discretization = rDG
  rdg_slope_reconstruction = none

  scaling_factor_2phase = '1 1e-3 1e-3 1e-8 1 1 1e-5'
[]

[FluidProperties]
  [./fp]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel2Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 0.1
    n_elems = 50

    A = 0.5

    f = 0.0
    f_interface = 0.0

    fp = fp

    initial_alpha_vapor = 0.5
    initial_p_liquid = 1e5
    initial_p_vapor  = 1e5
    initial_T_liquid = 349.144404977276
    initial_T_vapor  = 349.144404977276
    initial_vel_liquid = 0
    initial_vel_vapor  = 0

    pressure_relaxation = false
    velocity_relaxation = false
    interface_transfer  = false
    wall_mass_transfer  = false
  [../]

  [./inlet]
    type = InletDensityVelocity2Phase
    input = 'pipe:in'
    alpha_vapor = 0.5
    rho_liquid = 1168.39241168057
    rho_vapor  = 0.64046163283405
    vel_liquid = 0.1
    vel_vapor  = 0.2
  [../]

  [./outlet]
    type = Outlet2Phase
    input = 'pipe:out'
    p_liquid = 1e5
    p_vapor  = 1e5
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

  dt = 0.1
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]
