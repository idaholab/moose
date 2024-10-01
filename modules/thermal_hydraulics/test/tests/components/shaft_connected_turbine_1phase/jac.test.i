[GlobalParams]
  initial_p = 2e5
  initial_T = 500
  initial_vel = 100
  initial_vel_x = 100
  initial_vel_y = 0
  initial_vel_z = 0

  length = 1
  n_elems = 2
  A = 0.1
  A_ref = 0.1

  closures = simple_closures
  fp = fp
  f = 0.01
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 1.4
    cv = 725
    p_inf = 0
    q = 0
    q_prime = 0
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [sw1]
    type = SolidWall1Phase
    input = fch1:in
  []

  [fch1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    initial_p = 2e6
  []

  [turbine]
    type = ShaftConnectedTurbine1Phase
    inlet = 'fch1:out'
    outlet = 'fch2:in'
    position = '1 0 0'
    volume = 0.3
    inertia_coeff = '1 1 1 1'
    inertia_const = 1.61397
    speed_cr_I = 1e12
    speed_cr_fr = 0
    tau_fr_coeff = '0 0 12 0'
    tau_fr_const = 0
    omega_rated = 295
    D_wheel = 0.4
    head_coefficient = head
    power_coefficient = power
    use_scalar_variables = false
  []

  [fch2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
  []

  [sw2]
    type = SolidWall1Phase
    input = fch2:out
  []

  [shaft]
    type = Shaft
    connected_components = 'turbine'
    initial_speed = 300
  []

[]

[Functions]
  [head]
    type = PiecewiseLinear
    x = '0 0.1 1'
    y = '0 15 20'
  []
  [power]
    type = PiecewiseLinear
    x = '0 0.1 1'
    y = '0 0.05 0.18'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 0.001
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-snes_test_err'
  petsc_options_value = '1e-9'

  automatic_scaling = true

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10
[]
