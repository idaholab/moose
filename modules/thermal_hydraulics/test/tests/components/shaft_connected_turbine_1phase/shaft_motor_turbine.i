area = 0.2359
dt = 1.e-3

[GlobalParams]
  initial_p = 2e5
  initial_T = 600
  initial_vel = 100
  initial_vel_x = 100
  initial_vel_y = 0
  initial_vel_z = 0
  A = ${area}
  A_ref = ${area}
  f = 100
  scaling_factor_1phase = '0.04 0.04 0.04e-5'
  closures = simple_closures
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
  [turbine]
    type = ShaftConnectedTurbine1Phase
    inlet = 'pipe:out'
    outlet = 'pipe:in'
    position = '0 0 0'
    volume = 0.2
    inertia_coeff = '1 1 1 1'
    inertia_const = 1.61397
    speed_cr_I = 1e12
    speed_cr_fr = 0
    tau_fr_coeff = '0 0 0 0'
    tau_fr_const = 0
    omega_rated = 100
    D_wheel = 0.4
    head_coefficient = head
    power_coefficient = power
  []
  [pipe]
    type = FlowChannel1Phase
    position = '0.1 0 0'
    orientation = '1 0 0'
    length = 10
    n_elems = 20
    initial_p = 2e6
  []

  [dyno]
    type = ShaftConnectedMotor
    inertia = 1e2
    torque = -1e3
  []

  [shaft]
    type = Shaft
    connected_components = 'dyno turbine'
    initial_speed = 300
  []
[]


[Functions]
  [head]
    type = PiecewiseLinear
    x = '0 7e-3 1e-2'
    y = '0 15 20'
  []
  [power]
    type = PiecewiseLinear
    x = '0 6e-3 1e-2'
    y = '0 0.05 0.18'
  []

  [S_energy_fcn]
    type = ParsedFunction
    expression = '-(tau_driving+tau_fr)*omega'
    symbol_names = 'tau_driving tau_fr omega'
    symbol_values = 'turbine:driving_torque turbine:friction_torque shaft:omega'
  []
  [energy_conservation_fcn]
    type = ParsedFunction
    expression = '(E_change - S_energy * dt) / E_tot'
    symbol_names = 'E_change S_energy dt E_tot'
    symbol_values = 'E_change S_energy ${dt} E_tot'
  []
[]

[Postprocessors]
  # mass conservation
  [mass_pipes]
    type = ElementIntegralVariablePostprocessor
    variable = rhoA
    block = 'pipe'
    execute_on = 'initial timestep_end'
  []
  [mass_turbine]
    type = ScalarVariable
    variable = turbine:rhoV
    execute_on = 'initial timestep_end'
  []
  [mass_tot]
    type = SumPostprocessor
    values = 'mass_pipes mass_turbine'
    execute_on = 'initial timestep_end'
  []
  [mass_conservation]
    type = ChangeOverTimePostprocessor
    postprocessor = mass_tot
    change_with_respect_to_initial = true
    compute_relative_change = true
    execute_on = 'initial timestep_end'
  []

  # energy conservation
  [E_pipes]
    type = ElementIntegralVariablePostprocessor
    variable = rhoEA
    block = 'pipe'
    execute_on = 'initial timestep_end'
  []
  [E_turbine]
    type = ScalarVariable
    variable = turbine:rhoEV
    execute_on = 'initial timestep_end'
  []
  [E_tot]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 1'
    pp_names = 'E_pipes E_turbine'
    execute_on = 'initial timestep_end'
  []
  [S_energy]
    type = FunctionValuePostprocessor
    function = S_energy_fcn
    execute_on = 'initial timestep_end'
  []
  [E_change]
    type = ChangeOverTimePostprocessor
    postprocessor = E_tot
    execute_on = 'initial timestep_end'
  []

  # This should also execute on initial. This value is
  # lagged by one timestep as a workaround to moose issue #13262.
  [energy_conservation]
    type = FunctionValuePostprocessor
    function = energy_conservation_fcn
    execute_on = 'timestep_end'
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
  scheme = 'implicit-euler'

  dt = ${dt}
  num_steps = 6

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
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
[]
