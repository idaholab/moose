# Pump data used in this test comes from the Semiscale Program, summarized in NUREG/CR-4945

initial_T = 393.15
area = 1e-2
dt = 0.005

[GlobalParams]
  initial_p = 1.4E+07
  initial_T = ${initial_T}
  initial_vel = 0.01
  initial_vel_x = 0.01
  initial_vel_y = 0
  initial_vel_z = 0
  A = ${area}
  A_ref = ${area}
  f = 100
  scaling_factor_1phase = '1 1 1e-3'
  closures = simple_closures
  rdg_slope_reconstruction = minmod
  fp = fp
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pump]
    type = ShaftConnectedPump1Phase
    inlet = 'pipe:out'
    outlet = 'pipe:in'
    position = '0 0 0'
    scaling_factor_rhoEV = 1e-5
    volume = 0.3
    inertia_coeff = '1 1 1 1'
    inertia_const = 0.5
    omega_rated = 314
    speed_cr_I = 1e12
    speed_cr_fr = 0.001
    torque_rated = 47.1825
    volumetric_rated = 1
    head_rated = 58.52
    tau_fr_coeff = '4 0 80 0'
    tau_fr_const = 8
    head = head_fcn
    torque_hydraulic = torque_fcn
    density_rated = 124.2046
    use_scalar_variables = false
  []

  [pipe]
    type = FlowChannel1Phase
    position = '0.6096 0 0'
    orientation = '1 0 0'
    length = 10
    n_elems = 20
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
    data_file = semiscale_head_data.csv
    format = columns
  []
  [torque_fcn]
    type = PiecewiseLinear
    data_file = semiscale_torque_data.csv
    format = columns
  []
[]

[Postprocessors]
  [vel_avg]
    type = ElementAverageValue
    variable = vel
    block = 'pipe'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [hydraulic_torque]
    type = ElementAverageValue
    variable = hydraulic_torque
    block = 'pump'
    execute_on = 'initial timestep_end'
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

  dt = ${dt}
  num_steps = 40

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  velocity_as_vector = false
  file_base = 'pump_coastdown'
  [csv]
    type = CSV
    show = 'shaft:omega vel_avg'
  []
[]
