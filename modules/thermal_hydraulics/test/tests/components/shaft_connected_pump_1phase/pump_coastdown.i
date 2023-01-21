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

  [S_energy_fcn]
    type = ParsedFunction
    expression = '-tau_hyd * omega'
    symbol_names = 'tau_hyd  omega'
    symbol_values = 'pump:hydraulic_torque shaft:omega'
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
  [mass_pump]
    type = ScalarVariable
    variable = pump:rhoV
    execute_on = 'initial timestep_end'
  []
  [mass_tot]
    type = SumPostprocessor
    values = 'mass_pipes mass_pump'
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
  [E_pump]
    type = ScalarVariable
    variable = pump:rhoEV
    execute_on = 'initial timestep_end'
  []
  [E_tot]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 1'
    pp_names = 'E_pipes E_pump'
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
  exodus = true
[]
