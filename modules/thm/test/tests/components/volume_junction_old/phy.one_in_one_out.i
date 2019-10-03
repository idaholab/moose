[GlobalParams]
  initial_p = 1.0e5
  initial_vel = 1
  initial_T = 628.15
  scaling_factor_1phase = '1e4 1 1e-3'
  stabilization = supg
  closures = simple
[]

[Stabilizations]
  [./supg]
    type = SUPG
  [../]
[]

[FluidProperties]
  [./eos]
    type = LinearFluidProperties
    p_0 = 1e5        # Pa
    rho_0 = 865.51   # kg/m^3
    a2 = 5.7837e6    # m^2/s^2
    beta = 2.7524e-4 # K^{-1}
    cv =  1272.0     # at Tavg;
    e_0 = 7.9898e5   # J/kg
    T_0 = 628.15     # K
  [../]
[]

[Components]
  [./pipe1]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 0'
    orientation = '0 0 1'
    A = 3.14e-4
    D_h = 0.02
    f = 0.01
    length = 1
    n_elems = 10
  [../]

  [./pipe2]
    type = FlowChannel1Phase
    fp = eos
    # geometry
    position = '0 0 2'
    orientation = '0 0 1'
    A = 3.14e-4
    D_h = 0.02
    f = 0.01
    length = 1
    n_elems = 10
  [../]

  [./junction1]
    type = VolumeJunctionOld
    # geometry
    center = '0 0 1.5'
    connections = 'pipe1:out pipe2:in'
    K = '0.1 0.1'
    A_ref = 3.14e-2
    volume = 3.14e-2
    initial_T = 728.15
  [../]

  [./inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe1:in'
    rho = 865.51
    vel = 1.0
  [../]
  [./outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1.0e5
    legacy = true
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    petsc_options_iname = '-pc_factor_shift_type -pc_factor_shift_amount'
    petsc_options_value = ' NONZERO               1e-10'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1e-2
  num_steps = 10
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-7
  nl_max_its = 15

  l_tol = 1e-3
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
