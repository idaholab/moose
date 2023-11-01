# Operating conditions
u_inlet = 1
p_outlet = 2e5
p_init_md = 1e5
T_inlet = 320
T_initial = 300

# Need this id for the connection functor
outlet_md_elem_id = 100

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
    components = 1
  []
[]

[Functions]
  [inlet_fun]
    type = ParsedFunction
    expression = '${u_inlet} + 0.1 * min(t, 10)'
  []
  [inlet_fun_T]
    type = ParsedFunction
    expression = '${T_inlet} + 10 * t'
  []
[]

[Components]
  [comp1]
    type = FileMeshWCNSFVComponent
    file = rectangle.e
    position = '0 0 0'
    verbose = true

    add_flow_equations = true
    add_energy_equation = true
    add_scalar_equations = false
    turbulence_model = 'mixing-length'

    # Avoid naming conflicts between NS and THM
    velocity_variable = 'vx vy'
    density = 'rho_ns'

    inlet_boundaries = 'comp1:left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_functors = 'inlet_fun 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_functors = 'inlet_fun_T'

    wall_boundaries = 'comp1:top comp1:bottom'
    momentum_wall_types = 'noslip symmetry'
    energy_wall_types = 'heatflux'
    energy_wall_functors = '1'

    outlet_boundaries = 'comp1:right'

    initial_velocity = '${u_inlet} 0 0'
    initial_pressure = '${p_init_md}'

    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'

    # The junction adds more boundary conditions
    boundary_conditions_all_set = false
  []

  # Part 1b of the junction: use the boundary functors as boundary conditions to the 1D
  # solve.
  [junction]
    type = InletVelocityTemperature1PhaseFromWCNSFV
    input = 'comp2:in'
    vel = 'vx_outlet'
    T = T_outlet
    reversible = false
  []

  [comp2]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    gravity_vector = '0 0 0'
    length = 1.0
    n_elems = 50
    A = 1.0

    initial_T = ${T_initial}
    initial_p = ${p_outlet}
    initial_vel = ${u_inlet}

    f = 10000.0
    scaling_factor_1phase = '1 1 1e-5'
    closures = simple_closures
    fp = fp
  []

  [1d_outlet]
    type = Outlet1Phase
    input = 'comp2:out'
    p = ${p_outlet}
  []
[]

[FunctorMaterials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'mu k cp'
    prop_values = '1 1 1'
    block = 'comp1:body'
  []
  # Part 1a of the junction: create the boundary functors for integrating
  # the multiD outlet velocity
  [boundary_md_velocity_x]
    type = ADBoundaryIntegralFunctorMaterial
    functor_in = 'vx'
    # factor = '-1'
    functor_name = 'vx_outlet'
    integration_boundary = 'comp1:right'
  []
  [boundary_md_velocity_y]
    type = ADBoundaryIntegralFunctorMaterial
    functor_in = 'vy'
    functor_name = 'vy_outlet'
    integration_boundary = 'comp1:right'
  []
  [boundary_md_T]
    type = ADBoundaryIntegralFunctorMaterial
    functor_in = 'T_fluid'
    functor_name = 'T_outlet'
    integration_boundary = 'comp1:right'
  []
  # Part 2a of the junction: create the boundary functors for getting the pressure
  # value
  [define_pressure]
    type = GeneralFunctorFluidPropsVE
    fp = fp
    pressure_name = 'pressure_ad_elem'
    temperature_name = 'T_1d'
    # Loses derivatives
    # v = 'v'
    # e = 'e'
    # Good
    rhoA = 'rhoA'
    rhoEA = 'rhoEA'
    rhouA = 'rhouA'
    A = 'A'
    alpha = 1
    block = 'comp2'
  []
  # [smooth_pressure_onto_node]
  #   type = FunctorSmoother
  #   functors_in = 'pressure_ad_elem'
  #   functors_out = 'pressure_1d'
  #   smoothing_technique = 'layered_elem_average'
  # []
  # [boundary_1d_pressure]
  #   type = ADNodeValueFunctorMaterial
  #   functor_in = 'pressure_1d'
  #   functor_name = 'pressure_1d_left'
  #   nodeset = 'junction'
  #   subdomain_for_node = 'comp2'
  # []
  [boundary_1d_pressure]
    type = ADFixedElemValueFunctorMaterial
    functor_in = 'pressure_ad_elem' # TODO fix this
    # functor_in = 'p' # looses the derivative
    functor_name = 'pressure_1d_left'
    elem_id = ${outlet_md_elem_id}
  []

  # Density for NS
  [rho_ns]
    type = RhoFromPTFunctorMaterial
    fp = fp
    density_name = rho_ns
    pressure = 'pressure'
    temperature = 'T_fluid'
  []
[]

[FVBCs]
  # Part 2b of the junction, applying the outlet BC pressure
  [outlet-md]
    type = INSFVOutletPressureBC
    variable = pressure
    functor = 'pressure_1d_left'
    boundary = 'comp1:right'
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-5
  # automatic_scaling = true
  # off_diagonals_in_auto_scaling = true
  verbose = true
[]

[Debug]
  show_var_residual_norms = true
[]

[Problem]
  verbose_setup = true
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [p_inlet]
    type = SideAverageValue
    variable = pressure
    boundary = 'comp1:left'
  []
  [p_mid_mD]
    type = SideAverageValue
    variable = pressure
    boundary = 'comp1:right'
  []
  [p_mid_oneD]
    type = SideAverageValue
    variable = p
    boundary = 'comp2:in'
  []
  [p_out]
    type = SideAverageValue
    variable = p
    boundary = 'comp2:out'
  []
  [T_inlet]
    type = SideAverageValue
    variable = T_fluid
    boundary = 'comp1:left'
  []
  [T_mid_md]
    type = SideAverageValue
    variable = T_fluid
    boundary = 'comp1:right'
  []
  [T_mid_oneD]
    type = SideAverageValue
    variable = T
    boundary = 'comp2:in'
  []
  [T_out]
    type = SideAverageValue
    variable = T
    boundary = 'comp2:out'
  []
  [mdot_inlet]
    type = VolumetricFlowRate
    advected_quantity = 'rho_ns'
    vel_x = 'vx'
    boundary = 'comp1:left'
    rhie_chow_user_object = 'comp1:flow_ins_rhie_chow_interpolator'
  []
  [mdot_mid_md]
    type = VolumetricFlowRate
    advected_quantity = 'rho_ns'
    vel_x = 'vx'
    boundary = 'comp1:right'
    rhie_chow_user_object = 'comp1:flow_ins_rhie_chow_interpolator'
  []
  [mdot_mid_oneD]
    type = SideAverageValue
    variable = rhouA
    boundary = 'comp2:in'
  []
  [mdot_out_oneD]
    type = SideAverageValue
    variable = rhouA
    boundary = 'comp2:out'
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[AuxVariables]
  [T1d_chk]
    family = MONOMIAL
    order = CONSTANT
    block = comp2
    components = 1
  []
  [p_chk]
    family = MONOMIAL
    order = CONSTANT
    block = comp2
    components = 1
  []
[]

[AuxKernels]
  [p]
    type = FunctorAux
    variable = p_chk
    functor = 'pressure_ad_elem'
    execute_on = TIMESTEP_BEGIN
  []
  [T]
    type = FunctorAux
    variable = T1d_chk
    functor = 'T_1d'
  []
[]
