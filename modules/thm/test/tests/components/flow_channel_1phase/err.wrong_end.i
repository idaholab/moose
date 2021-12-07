[GlobalParams]
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1. 1e-4'

  closures = simple_closures
[]

[FluidProperties]
  [barotropic]
    type = LinearFluidProperties
    p_0 = 1.e5     # Pa
    rho_0 = 1.e3   # kg/m^3
    a2 = 1.e7      # m^2/s^2
    beta = .46e-3 # K^{-1}
    cv = 4.18e3    # J/kg-K, could be a global parameter?
    e_0 = 1.254e6  # J/kg
    T_0 = 300      # K
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
    fp = barotropic
    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    A = 1.
    D_h = 1.12837916709551
    f = 0.01
    length = 1
    n_elems = 100
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe:asdf'      # this is an error we are checking for
    p0 = 1e5
    T0 = 300
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 9.5e4
  []
[]

[Preconditioning]
  [FDP_PJFNK]
    type = FDP
    full = true

    petsc_options_iname = '-mat_fd_coloring_err'
    petsc_options_value = '1.e-10'
  []
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
[]


[Outputs]
  [out]
    type = Exodus
  []
[]
