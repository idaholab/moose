L = 30
nx = 600
bulk_u = 0.01
p_ref = 101325.0
T_in = 860.
q_source = 20000.
advected_interp_method = 'upwind'

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = ${L}
    nx = ${nx}
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'weakly-compressible'

        velocity_variable = 'vel_x'

        density = 'rho'
        dynamic_viscosity = 'mu'

        initial_velocity = '${bulk_u} 0 0'
        initial_pressure = '${p_ref}'

        inlet_boundaries = 'left'
        # momentum_inlet_types = 'fixed-velocity'
        # momentum_inlet_functors = '${bulk_u} 0'
        momentum_inlet_types = 'flux-velocity'
        flux_inlet_pps = '${bulk_u}'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '${p_ref}'

        momentum_advection_interpolation = ${advected_interp_method}
        pressure_two_term_bc_expansion = false
        momentum_two_term_bc_expansion = false
      []
    []
    [FluidHeatTransfer]
      [energy]
        coupled_flow_physics = flow

        solve_for_enthalpy = true
        # There is no fluid temperature (auxiliary) variable when solving for enthalpy
        # with nonlinear finite volume, because we need T_from_p_h to be computed on-the-fly
        # rather than on an auxiliary kernel update which does not preserve automatic differentiation
        # fluid_temperature_variable = 'T_fluid'

        fp = 'lead'
        thermal_conductivity = 'k'
        specific_heat = 'cp'

        initial_enthalpy = '${fparse 800 * 240}'

        energy_inlet_types = 'flux-velocity'
        # specifies inlet temperature, not inlet enthalpy
        energy_inlet_functors = '${T_in}'

        # Source term
        external_heat_source = source_func

        # Numerical scheme
        energy_advection_interpolation = ${advected_interp_method}
        energy_two_term_bc_expansion = true
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
    T_fluid = 'T_fluid'
    speed = 1
    porosity = 1
    characteristic_length = 1
  []
  [source_func]
    type = ADParsedFunctorMaterial
    property_name = source_func
    functor_names = 'rho'
    expression = ${q_source}
  []
[]

[AuxVariables]
  [T_out]
    type = MooseVariableFVReal
    [AuxKernel]
      type = FunctorAux
      functor = 'T_fluid'
    []
  []
[]

[Postprocessors]
  [T_out_sim]
    type = PointValue
    variable = T_out
    point = '${fparse L * (nx-0.5)/ nx} 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
[]

[Outputs]
  [out]
    type = CSV
    hide = 'area_pp_left'
  []
[]
