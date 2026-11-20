# Physics syntax version of porous-baffle-2d-enthalpy.i: steady conjugate heat transfer from a
# heated solid to fluid in a 0.5-m porous block bounded by two pressure baffles. The flow, fluid
# energy and solid energy Physics create the objects of the explicit setup, so this input
# reproduces its results.

mu = 2e-3
rho = 1000
u_in = 2

cp_fluid = 4200
k_fluid = 0.4
k_solid = 0.2
h_vol = 30000
q_solid = 20000000
T_inlet = 300
T_initial = 300

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5 0.5'
    dy = '0.5'
    ix = '10 10 10'
    iy = '3'
    subdomain_id = '1 2 3'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'baffle'
  []
  [baffle2]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'baffle2'
  []
  [split_top_bottom]
    type = BreakBoundaryOnSubdomainGenerator
    input = baffle2
    boundaries = 'top bottom'
  []
  [delete]
    type = BoundaryDeletionGenerator
    boundary_names = 'top bottom'
    input = split_top_bottom
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        velocity_variable = 'superficial_u superficial_v'
        pressure_variable = 'pressure'

        density = ${rho}
        dynamic_viscosity = ${mu}

        porous_medium_treatment = true
        porosity = porosity
        porosity_interface_pressure_treatment = bernoulli
        pressure_drop_sidesets = 'baffle baffle2'
        pressure_drop_form_factors = '0 0'
        pressure_drop_reference_velocity_side = 'lower_porosity higher_porosity'
        pressure_jump_relaxation = 0.1
        pressure_gradient_relaxation = 1.0

        friction_blocks = '2'
        friction_types = 'Forchheimer'
        friction_coeffs = 'forch'

        initial_velocity = '${u_in} 0 0'
        initial_pressure = 0

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${u_in} 0'
        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'
        wall_boundaries = 'top_to_1 top_to_2 top_to_3 bottom_to_1 bottom_to_2 bottom_to_3'
        momentum_wall_types = 'symmetry symmetry symmetry symmetry symmetry symmetry'

        momentum_advection_interpolation = 'upwind'
        momentum_two_term_bc_expansion = false
        # No extrapolated pressure at the fixed-velocity inlet, as in the explicit setup
        pressure_two_term_bc_expansion = false
        orthogonality_correction = false
      []
    []
    [FluidHeatTransferSegregated]
      [fluid]
        fluid_temperature_variable = 'T_fluid'
        solve_for_enthalpy = true
        specific_heat = 'cp_fluid'
        thermal_conductivity = ${k_fluid}
        # Matches the explicit setup, which uses k / cp without a porosity factor
        effective_conductivity = true
        initial_temperature = ${T_initial}

        energy_inlet_types = 'fixed-temperature'
        energy_inlet_functors = '${T_inlet}'
        energy_wall_types = 'heatflux heatflux heatflux heatflux heatflux heatflux'
        energy_wall_functors = '0 0 0 0 0 0'

        ambient_convection_blocks = '2'
        ambient_convection_alpha = 'h_vol'
        ambient_temperature = 'T_solid'

        energy_advection_interpolation = 'upwind'
        energy_two_term_bc_expansion = false
        use_nonorthogonal_correction = false
      []
    []
    [SolidHeatTransferSegregated]
      [solid]
        block = 2
        porosity = porosity
        solid_temperature_variable = 'T_solid'
        fluid_temperature_variable = 'T_fluid'
        rho_solid = 2000
        cp_solid = 500
        thermal_conductivity_solid = ${k_solid}
        initial_temperature = ${T_initial}

        ambient_convection_blocks = '2'
        ambient_convection_alpha = 'h_vol'
        ambient_convection_temperature = 'T_fluid'

        external_heat_source = 'q_solid_scaled'

        use_nonorthogonal_correction = false
      []
    []
  []
[]

[FunctorMaterials]
  [forch]
    type = GenericVectorFunctorMaterial
    prop_names = forch
    prop_values = '0 0 0'
  []
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 1.0 2 0.5 3 1.0'
  []
  [fluid_constants]
    type = GenericFunctorMaterial
    prop_names = 'cp_fluid h_vol'
    prop_values = '${cp_fluid} ${h_vol}'
  []
  [q_solid]
    type = GenericFunctorMaterial
    prop_names = 'q_solid'
    prop_values = ${q_solid}
    block = 2
  []
  # The heat source density per unit total volume is deposited in the solid fraction
  [q_solid_scaled]
    type = ParsedFunctorMaterial
    property_name = q_solid_scaled
    functor_names = 'porosity q_solid'
    expression = '(1 - porosity) * q_solid'
    block = 2
  []
  [h_from_p_T_functor]
    type = ParsedFunctorMaterial
    property_name = h_from_p_T_functor
    functor_names = 'T_fluid'
    expression = '${cp_fluid} * T_fluid'
  []
  [T_from_p_h_functor]
    type = ParsedFunctorMaterial
    property_name = T_from_p_h_functor
    functor_names = 'h'
    expression = 'h / ${cp_fluid}'
  []
[]

[Postprocessors]
  [u_block_2]
    type = ElementAverageValue
    variable = superficial_u
    block = 2
  []
  [T_fluid_out]
    type = SideAverageValue
    variable = T_fluid
    boundary = right
  []
  [T_fluid_block_2]
    type = ElementAverageValue
    variable = T_fluid
    block = 2
  []
  [T_solid_block_2]
    type = ElementAverageValue
    variable = T_solid
    block = 2
  []
  [h_fluid_out]
    type = SideAverageValue
    variable = h
    boundary = right
  []
[]

[VectorPostprocessors]
  # Sample each x-cell once at its center.
  [centerline]
    type = LineValueSampler
    variable = 'superficial_u T_fluid T_solid'
    start_point = '0.0119047619047619 0.25 0'
    end_point = '1.4880952380952381 0.25 0'
    num_points = 30
    sort_by = x
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  energy_l_abs_tol = 1e-14
  solid_energy_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  solid_energy_l_tol = 0
  rhie_chow_user_object = pins_rhie_chow_interpolator
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  energy_system = energy_system
  solid_energy_system = solid_energy_system
  momentum_equation_relaxation = 0.2
  pressure_variable_relaxation = 0.1
  energy_equation_relaxation = 0.8
  num_iterations = 1000
  # See porous-baffle-2d-enthalpy.i for the choice of the outer tolerances
  pressure_absolute_tolerance = 1e-6
  momentum_absolute_tolerance = 1e-2
  energy_absolute_tolerance = 1e-8
  solid_energy_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
