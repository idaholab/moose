# This test tests that mass and energy are conserved.

dt = 1.e-2
head = 95.
volume = 1.
A = 1.
g = 9.81

[GlobalParams]
  initial_T = 393.15
  initial_vel = 0
  f = 0
  fp = fp
  scaling_factor_1phase = '0.04 0.04 0.04e-5'
  closures = simple_closures
  A = ${A}
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
  [wall_in]
    type = SolidWall1Phase
    input = 'pipe1:in'
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    initial_p = 1.7E+07
    n_elems = 10
    gravity_vector = '0 0 0'
  []

  [pump]
    type = Pump1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1.02 0 0'
    initial_p = 1.3e+07
    scaling_factor_rhoEV = 1e-5
    head = ${head}
    A_ref = ${A}
    volume = ${volume}
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1.04 0 0'
    orientation = '1 0 0'
    length = 1
    initial_p = 1.3e+07
    n_elems = 10
    gravity_vector = '0 0 0'
  []

  [wall_out]
    type = SolidWall1Phase
    input = 'pipe2:out'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'
  start_time = 0
  dt = ${dt}
  num_steps = 6
  abort_on_solve_fail = true
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

[Postprocessors]
  # mass conservation
  [mass_pipes]
    type = ElementIntegralVariablePostprocessor
    variable = rhoA
    block = 'pipe1 pipe2'
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
  [mass_tot_change]
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
    block = 'pipe1 pipe2'
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

  # this should also execute on initial, this value is
  # lagged by one timestep as a workaround to moose issue #13262
  [E_conservation]
    type = FunctionValuePostprocessor
    function = E_conservation_fcn
    execute_on = 'timestep_end'
  []
[]

[Functions]
  [S_energy_fcn]
    type = ParsedFunction
    expression = 'rhouV * g * head * A / volume'
    symbol_names = 'rhouV g head A volume'
    symbol_values = 'pump:rhouV ${g} ${head} ${A} ${volume}'
  []
  [E_conservation_fcn]
    type = ParsedFunction
    expression = '(E_change - S_energy * dt) / E_tot'
    symbol_names = 'E_change S_energy dt E_tot'
    symbol_values = 'E_change S_energy ${dt} E_tot'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
    show = 'mass_tot_change E_conservation'
  []
[]
