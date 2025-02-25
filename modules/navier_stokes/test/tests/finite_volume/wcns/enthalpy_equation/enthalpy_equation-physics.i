H = 0.015
L = 1
bulk_u = 0.01
p_ref = 101325.0

advected_interp_method = 'upwind'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = -${H}
    ymax = ${H}
    nx = 30
    ny = 15
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'weakly-compressible'

        velocity_variable = 'vel_x vel_y'

        density = 'rho'
        dynamic_viscosity = 'mu'

        initial_velocity = '${bulk_u} 0 0'
        initial_pressure = '${p_ref}'

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${bulk_u} 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '${p_ref}'

        orthogonality_correction = false
        momentum_advection_interpolation = ${advected_interp_method}
        pressure_two_term_bc_expansion = false
        momentum_two_term_bc_expansion = false
      []
    []
    [FluidHeatTransferSegregated]
      [energy]
        coupled_flow_physics = flow

        solve_for_enthalpy = true
        fluid_temperature_variable = 'T'

        fp = 'lead'
        thermal_conductivity = 'k'
        specific_heat = 'cp'

        initial_temperature = '777'
        initial_enthalpy = '44000'

        energy_inlet_types = 'fixed-temperature'
        energy_inlet_functors = '860'
        energy_wall_types = 'fixed-temperature fixed-temperature'
        energy_wall_functors = '950 950'

        energy_advection_interpolation = ${advected_interp_method}
        energy_two_term_bc_expansion = false
        use_nonorthogonal_correction = false
      []
    []
  []
[]

[FluidProperties]
  [lead]
    type = LeadFluidProperties
  []
[]

[FunctorMaterials]
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = lead
    pressure = ${p_ref}
    T_fluid = 'T'
    speed = 1
    porosity = 1
    characteristic_length = 1
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-6
  pressure_l_abs_tol = 1e-6
  energy_l_abs_tol = 1e-8
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9
  num_iterations = 200
  pressure_absolute_tolerance = 1e-6
  momentum_absolute_tolerance = 1e-6
  energy_absolute_tolerance = 1e-6
  print_fields = false
  momentum_l_max_its = 1000

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'

  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = 'TIMESTEP_BEGIN FINAL'
[]

# To match the gold file
[AuxVariables]
  [rho_var]
    type = MooseLinearVariableFVReal
  []
  [cp_var]
    type = MooseLinearVariableFVReal
  []
  [mu_var]
    type = MooseLinearVariableFVReal
  []
  [k_var]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [rho_out]
    type = FunctorAux
    functor = 'rho'
    variable = 'rho_var'
    execute_on = 'NONLINEAR'
  []
  [cp_out]
    type = FunctorAux
    functor = 'cp'
    variable = 'cp_var'
    execute_on = 'NONLINEAR'
  []
  [mu_out]
    type = FunctorAux
    functor = 'mu'
    variable = 'mu_var'
    execute_on = 'NONLINEAR'
  []
  [k_out]
    type = FunctorAux
    functor = 'k'
    variable = 'k_var'
    execute_on = 'NONLINEAR'
  []
[]
