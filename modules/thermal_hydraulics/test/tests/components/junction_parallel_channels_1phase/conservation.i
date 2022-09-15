# Junction between 2 pipes where the second has half the area of the first.
# The momentum density of the second should be twice that of the first.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 300
  initial_p = 1e5
  initial_vel = 20
  initial_vel_x = 20
  initial_vel_y = 0
  initial_vel_z = 0

  f = 0

  fp = eos

  scaling_factor_1phase = '1 1e-2 1e-5'

  closures = simple_closures
[]

[FluidProperties]
  [eos]
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
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    A = 1
    n_elems = 20
  []

  [junction1]
    type = JunctionParallelChannels1Phase
    connections = 'pipe1:out pipe2:in'
    scaling_factor_rhouV = 1e-4
    scaling_factor_rhoEV = 1e-5
    position = '1 0 0'
    volume = 1e-2
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    A = 0.5
    n_elems = 20
  []

  [junction2]
    type = JunctionParallelChannels1Phase
    connections = 'pipe2:out pipe1:in'
    scaling_factor_rhouV = 1e-4
    scaling_factor_rhoEV = 1e-5
    position = '1 0 0'
    volume = 1e-2
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
  scheme = 'bdf2'

  start_time = 0
  dt = 0.05
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = basic
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 20

  l_tol = 1e-3
  l_max_its = 20

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
  [mass_junction1]
    type = ScalarVariable
    variable = junction1:rhoV
    execute_on = 'initial timestep_end'
  []
  [mass_junction2]
    type = ScalarVariable
    variable = junction2:rhoV
    execute_on = 'initial timestep_end'
  []
  [mass_tot]
    type = SumPostprocessor
    values = 'mass_pipes mass_junction1 mass_junction2'
    execute_on = 'initial timestep_end'
  []
  [mass_tot_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = mass_tot
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
  [E_junction1]
    type = ScalarVariable
    variable = junction1:rhoEV
    execute_on = 'initial timestep_end'
  []
  [E_junction2]
    type = ScalarVariable
    variable = junction2:rhoEV
    execute_on = 'initial timestep_end'
  []
  [E_tot]
    type = SumPostprocessor
    values = 'E_pipes E_junction1 E_junction2'
    execute_on = 'initial timestep_end'
  []
  [E_tot_change]
    type = ChangeOverTimePostprocessor
    change_with_respect_to_initial = true
    postprocessor = E_tot
    compute_relative_change = true
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  [out]
    type = CSV
    show = 'mass_tot_change E_tot_change'
  []
[]
