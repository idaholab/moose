T_in = 300. # K
P_out = 250 # Pa
length = 4.0

[GlobalParams]
  initial_p = ${P_out}
  initial_vel = 0.0001
  initial_T = ${T_in}
  gravity_vector = '0 0 0'
  rdg_slope_reconstruction = minmod
  scaling_factor_1phase = '1 1e-2 1e-4'
  closures = thm_closures
  fp = he
[]

[FluidProperties]
  [he]
    type = IdealGasFluidProperties
    molar_mass = 4e-3
    gamma = 1.67
    k = 0.2556
    mu = 3.22639e-5
  []
[]

[Closures]
  [thm_closures]
    type = Closures1PhaseTHM
  []
[]

[Components]

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe_inlet:in'
    T0 = 300.0
    p0 = 310.27986859007
  []

  [pipe_inlet]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = ${length}
    n_elems = 1
    A = 0.007853981633974483
    D_h = 0.1
  []

  [junction_pipe_inlet_to_pipe]
    type = JunctionOneToOne1Phase
    connections = 'pipe_inlet:out pipe:in'
  []

  [pipe]
    type = FlowChannel1PhaseWithAddedMass
    position = '0 0 0'
    orientation = '1 0 0'
    length = ${length}
    n_elems = 1
    A = 0.007853981633974483
    D_h = 0.1
  []

  [junction_pipe_to_pipe_outlet]
    type = JunctionOneToOne1Phase
    connections = 'pipe:out pipe_outlet:in'
  []

  [pipe_outlet]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = ${length}
    n_elems = 1
    A = 0.007853981633974483
    D_h = 0.1
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe_outlet:out'
    p = ${P_out}
  []
[]

[AuxVariables]
  [mass_flux]
    type = MooseVariableFVReal
  []
  [rho_var]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [populate_mass_flux]
    type = ParsedAux
    variable = mass_flux
    coupled_variables = 'rho_var vel_x'
    expression = 'rho_var * vel_x'
  []
  [poplate_rho_var]
    type = FunctorAux
    functor = 'rho'
    variable = 'rho_var'
  []
[]

[Functions]
  [massflux_outlet]
    type = ParsedFunction
    expression = '0.05'
  []
[]

[Postprocessors]
  [pipe_p_in]
    type = SideAverageValue
    boundary = pipe:in
    variable = p
  []

  [pipe_p_out]
    type = SideAverageValue
    boundary = pipe:out
    variable = p
  []

  [pipe_delta_p]
    type = ParsedPostprocessor
    pp_names = 'pipe_p_in pipe_p_out'
    function = 'pipe_p_in - pipe_p_out'
  []

  [pipe_T_in]
    type = SideAverageValue
    boundary = pipe:in
    variable = T
  []

  [pipe_T_out]
    type = SideAverageValue
    boundary = pipe:out
    variable = T
  []

  [pipe_v_in]
    type = SideAverageValue
    boundary = pipe:in
    variable = vel_x
  []

  [pipe_v_out]
    type = SideAverageValue
    boundary = pipe:out
    variable = vel_x
  []

  [mass_flux_in]
    type = SideAverageValue
    boundary = pipe_inlet:out
    variable = mass_flux
  []

  [mass_flux_out]
    type = SideAverageValue
    boundary = pipe_outlet:in
    variable = mass_flux
  []

  [mass_imbalance_source_pp]
    type = Receiver
    default = 1e-4
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
  solve_type = NEWTON
  line_search = basic

  start_time = 0
  end_time = 0.1

  dt = 0.001

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 25
[]

[Outputs]
  exodus = false
  [csv]
    type = CSV
    execute_on = 'INITIAL TIMESTEP_END'
    file_base = 'pipe_added_mass'
  []
  print_linear_residuals = false
[]
