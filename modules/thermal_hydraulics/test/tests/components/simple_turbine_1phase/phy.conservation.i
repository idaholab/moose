[GlobalParams]
  initial_p = 1e6
  initial_T = 517
  initial_vel = 4.3
  initial_vel_x = 4.3
  initial_vel_y = 0
  initial_vel_z = 0

  fp = fp

  closures = simple_closures
  f = 0

  rdg_slope_reconstruction = minmod
  gravity_vector = '0 0 0'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.01
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = 10
    T = 517
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 1
  []

  [turbine]
    type = SimpleTurbine1Phase
    connections = 'pipe1:out pipe2:in'
    position = '1 0 0'
    volume = 1
    A_ref = 1.0
    K = 0
    on = true
    power = 1000
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '1. 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 1
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = 1e6
  []
[]

[Postprocessors]
  [mass_in]
    type = ADFlowBoundaryFlux1Phase
    equation = mass
    boundary = inlet
  []
  [mass_out]
    type = ADFlowBoundaryFlux1Phase
    equation = mass
    boundary = outlet
  []
  [mass_diff]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 -1'
    pp_names = 'mass_in mass_out'
  []
  [p_in]
    type = SideAverageValue
    boundary = pipe1:in
    variable = p
  []
  [vel_in]
    type = SideAverageValue
    boundary = pipe1:in
    variable = vel_x
  []
  [momentum_in]
    type = ADFlowBoundaryFlux1Phase
    equation = momentum
    boundary = inlet
  []
  [momentum_out]
    type = ADFlowBoundaryFlux1Phase
    equation = momentum
    boundary = outlet
  []
  [dP]
    type = ParsedPostprocessor
    pp_names = 'p_in W_dot'
    function = 'p_in * (1 - (1-W_dot/(10*2910.06*517))^(1.4/0.4))'
  []
  [momentum_diff]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 -1 -1'
    pp_names = 'momentum_in momentum_out dP' # momentum source = -dP * A and A=1
  []

  [energy_in]
    type = ADFlowBoundaryFlux1Phase
    equation = energy
    boundary = inlet
  []
  [energy_out]
    type = ADFlowBoundaryFlux1Phase
    equation = energy
    boundary = outlet
  []
  [W_dot]
    type = ScalarVariable
    variable = turbine:W_dot
  []
  [energy_diff]
    type = LinearCombinationPostprocessor
    pp_coefs = '1 -1 -1'
    pp_names = 'energy_in energy_out W_dot'
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
  scheme = bdf2

  start_time = 0
  end_time = 10
  dt = 0.5

  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'

  nl_rel_tol = 1e-7
  nl_abs_tol = 2e-6

  nl_max_its = 10
  l_tol = 1e-3
[]

[Outputs]
  [csv]
    type = CSV
    show = 'mass_diff energy_diff momentum_diff'
    execute_on = 'final'
  []
[]
