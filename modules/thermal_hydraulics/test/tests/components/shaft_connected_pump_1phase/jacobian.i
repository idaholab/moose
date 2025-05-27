# Pump data used in this test comes from the LOFT Systems Tests, described in NUREG/CR-0247

[GlobalParams]
  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

  closures = simple_closures
  fp = fp
  f = 0
[]

[FluidProperties]
  [fp]
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
  [fch1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2
    A = 1
  []

  [pump]
    type = ShaftConnectedPump1Phase
    inlet = 'fch1:out'
    outlet = 'fch2:in'
    position = '1 0 0'
    volume = 0.3
    A_ref = 1
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
    inertia_coeff = '1 1 1 1'
    inertia_const = 1.61397
    omega_rated = 314
    speed_cr_I = 1e12
    speed_cr_fr = 0
    torque_rated = 47.1825
    volumetric_rated = 1
    head_rated = 58.52
    tau_fr_coeff = '0 0 9.084 0'
    tau_fr_const = 0
    head = head_fcn
    torque_hydraulic = torque_fcn
    density_rated = 1
  []

  [fch2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2
    A = 1
  []

  [shaft]
    type = Shaft
    connected_components = 'pump'
    initial_speed = 1
  []

[]

[Functions]
  [head_fcn]
    type = PiecewiseLinear
    data_file = loft_head_data.csv
    format = columns
  []
  [torque_fcn]
    type = PiecewiseLinear
    data_file = loft_torque_data.csv
    format = columns
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
  scheme = 'bdf2'

  start_time = 0
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-snes_test_err'
  petsc_options_value = '2e-10'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10
[]
