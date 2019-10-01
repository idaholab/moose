[GlobalParams]
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1. 1. 1.'
  closures = simple
[]

[FluidProperties]
  [./eos]
    type = LinearFluidProperties
    p_0 = 1.e5     # Pa
    rho_0 = 1.e3   # kg/m^3
    a2 = 1.e7      # m^2/s^2
    beta = .46e-3  # K^{-1}
    cv = 4.18e3    # J/kg-K, could be a global parameter?
    e_0 = 1.254e6  # J/kg
    T_0 = 300      # K
  [../]
[]

[HeatStructureMaterials]
  [./fuel-mat]
    type = SolidMaterialProperties
    k = 2.5
    Cp = 300.
    rho = 1.032e4
  [../]
  [./gap-mat]
    type = SolidMaterialProperties
    k = 0.6
    Cp = 1.
    rho = 1.
  [../]
  [./clad-mat]
    type = SolidMaterialProperties
    k = 21.5
    Cp = 350.
    rho = 6.55e3
  [../]
[]

[Components]
  [./reactor]
    type = TotalPower
    power = 77337.69407
  [../]

  [./pipe1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '0 0 1'
    A = 8.78882e-5  #PWR, A = pitch^2 - PI * D_fuel * D_fuel / 4, pitch = 12.6 mm, D_fuel = 9.5 mm
    D_h = 0.01179
    length = 3.865
    n_elems = 20

    f = 0.01
  [../]

  [./junction]
    type = VolumeJunctionOld
    connections = 'pipe1:out pipe2:in'
    A_ref = 1
    center = '0 0 0'
    volume = 1
    K = '0'         # this is wrong
  [../]

  [./pipe1]
    type = FlowChannel1Phase
    A = 1
    D_h = 1
    length = 1
    n_elems = 1
    orientation = '1 0 0'
    position = '0 0 1'
    fp = eos
  [../]

  [./inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'CCH1:in'
    p0 = 1
    T0 = 1
  [../]
  [./outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = 1
    legacy = true
  [../]
[]

[Preconditioning]
  [./FDP_PJFNK]
    type = FDP
    full = true

    petsc_options_iname = '-mat_fd_coloring_err'
    petsc_options_value = '1.e-10'
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
  num_steps = 10 # The number of timesteps in a transient run

  [./Quadrature]
    type = TRAP

    # Specify the order as FIRST, otherwise you will get warnings in DEBUG mode...
    order = FIRST
  [../]
[]


[Outputs]
  [./out]
    type = Exodus
  [../]
[]
