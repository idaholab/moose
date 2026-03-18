##########################################################
# Simulation of Gallium Melting Experiment (Enthalpy form)
# Ref: Gau, C., & Viskanta, R. (1986). Melting and solidification of a pure metal on a vertical wall.
#
# Energy variable: specific enthalpy (h) with h(T_solidus)=0.
# Temperature is derived from h through INSFVPhaseChangeEnthalpyFunctorMaterial.
##########################################################

# Properties evaluated at T_solidus (degC). cp correlations are written in Kelvin.
mu = ${fparse 0.0156 - 1.053e-4 * T_solidus + 3.198e-7 * T_solidus^2 - 5.005e-10 * T_solidus^3}
rho_liquid = ${fparse 6273.98 - 0.605*T_solidus + 4.82e-5 * T_solidus^2}
L = 80160
alpha_b = 8.26e-5
T_solidus = 29.78
T_liquidus = '${fparse T_solidus + 0.1}'
advected_interp_method = 'upwind'
T_cold = 28.0
T_hot = 38.0
Nx = 300
Ny = 150

# Convenience: cp values evaluated at (T_solidus + 273.15)
cp_solid_val = ${fparse -9858.85 + 64.57*(T_solidus+273.15) - 0.1014*(T_solidus+273.15)^2}
cp_liquid_val = ${fparse 828.32 - 3.056*(T_solidus+273.15) + 0.0079*(T_solidus+273.15)^2 - 9.13e-6*(T_solidus+273.15)^3}

# Wall enthalpies (h(T_solidus)=0)
h_cold = ${fparse cp_solid_val*(T_cold - T_solidus)}
h_hot  = ${fparse L + cp_liquid_val*(T_hot - T_liquidus)}

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 88.9e-3
    ymin = 0
    ymax = 63.5e-3
    nx = ${Nx}
    ny = ${Ny}
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = 'rho_l'
    p_diffusion_kernel = p_diffusion
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[AuxVariables]
  [U]
    type = MooseLinearVariableFVReal
  []
  [fl]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
  []
  [T]
    type = MooseLinearVariableFVReal
    initial_condition = ${T_cold}
  []
  [density]
    type = MooseLinearVariableFVReal
  []
  [th_cond]
    type = MooseLinearVariableFVReal
  []
  [cp_var]
    type = MooseLinearVariableFVReal
  []
  [darcy_coef]
    type = MooseLinearVariableFVReal
  []
  [fch_coef]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []

  # Output helpers (temperature & liquid fraction are *derived* from enthalpy)
  [T_from_h]
    type = FunctorAux
    functor = 'T_from_p_h'
    variable = 'T'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [fl_from_h]
    type = FunctorAux
    functor = 'liquid_fraction'
    variable = 'fl'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [rho_out]
    type = FunctorAux
    functor = 'rho_mixture'
    variable = 'density'
  []
  [th_cond_out]
    type = FunctorAux
    functor = 'k_mixture'
    variable = 'th_cond'
  []
  [cp_out]
    type = FunctorAux
    functor = 'cp_mixture'
    variable = 'cp_var'
  []
  [darcy_out]
    type = FunctorAux
    functor = 'Darcy_coefficient'
    variable = 'darcy_coef'
  []
  [fch_out]
    type = FunctorAux
    functor = 'Forchheimer_coefficient'
    variable = 'fch_coef'
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.5
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.2
  []
  [h]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    # Reference: h(T_solidus) = 0.0
    initial_condition = '${h_cold}'
  []
[]

[LinearFVKernels]
  [u_time]
    type = LinearFVTimeDerivative
    variable = vel_x
    factor = 'rho_mixture'
  []
  [v_time]
    type = LinearFVTimeDerivative
    variable = vel_y
    factor = 'rho_mixture'
  []

  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = '${mu}'
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = '${mu}'
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []

  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []

  [u_boussinesq]
    type = LinearFVMomentumBoussinesq
    variable = vel_x
    rho = '${rho_liquid}'
    gravity = '0 -9.81 0'
    alpha_name = ${alpha_b}
    ref_temperature = ${T_cold}
    # LinearFVMomentumBoussinesq expects a *variable* name (not a functor).
    # Our output temperature variable is T (filled from T_from_p_h above).
    T_fluid = T
    momentum_component = 'x'
  []
  [v_boussinesq]
    type = LinearFVMomentumBoussinesq
    variable = vel_y
    rho = '${rho_liquid}'
    gravity = '0 -9.81 0'
    alpha_name = ${alpha_b}
    ref_temperature = ${T_cold}
    T_fluid = T
    momentum_component = 'y'
  []

  [u_friction_darcy]
    type = LinearFVReaction
    variable = vel_x
    coeff = 'darcy_coef_friction'
  []
  [v_friction_darcy]
    type = LinearFVReaction
    variable = vel_y
    coeff = 'darcy_coef_friction'
  []
  [u_friction_forch]
    type = LinearFVReaction
    variable = vel_x
    coeff = 'forch_coef_friction'
  []
  [v_friction_forch]
    type = LinearFVReaction
    variable = vel_y
    coeff = 'forch_coef_friction'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []

  [h_time]
    type = LinearFVTimeDerivative
    variable = h
    factor = 'rho_mixture'
  []
  [h_advection]
    type = LinearFVEnergyAdvection
    variable = h
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [conduction]
    type = LinearFVDiffusion
    variable = h
    diffusion_coeff = 'kappa_h'
    use_nonorthogonal_correction = false
  []
[]

[LinearFVBCs]
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = vel_y
    functor = 0.0
  []

  [hot_wall]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = h
    functor = '${h_hot}'
    boundary = 'left'
  []
  [cold_wall]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = h
    functor = '${h_cold}'
    boundary = 'right'
  []
[]

[FunctorMaterials]
  # Phase-change mapping h <-> T and derived fields.
  [phase_change_enthalpy]
    type = INSFVPhaseChangeEnthalpyFunctorMaterial
    cp_solid = cp_s
    cp_liquid = cp_l
    L = ${L}
    T_solidus = ${T_solidus}
    T_liquidus = ${T_liquidus}

    # This 'temperature' input is used only for h_from_p_T. We do not use h_from_p_T
    # in this file (wall enthalpies are prescribed directly), so a dummy constant is fine.
    temperature = '${T_solidus}'

    enthalpy = h
  []

  # Rewrite conduction in terms of h:  -k*grad(T) = -(k*dTdh)*grad(h)
  [kappa_h]
    type = ParsedFunctorMaterial
    property_name = 'kappa_h'
    functor_names = 'k_mixture dTdh'
    functor_symbols = 'k dTdh'
    expression = 'k*dTdh'
  []

  # Mixture properties
  [eff_props]
    # Use the linear-FV mixture functor material (Real-only) to avoid AD dependencies.
    type = WCNSLinearFVMixtureFunctorMaterial
    phase_2_names = 'cp_s k_s rho_s'
    phase_1_names = 'cp_l k_l rho_l'
    prop_names = 'cp_mixture k_mixture rho_mixture'
    phase_1_fraction = liquid_fraction
  []

  # Mushy-zone resistance
  [mushy_zone_resistance]
    type = INSFVMushyPorousFrictionFunctorMaterial
    liquid_fraction = 'liquid_fraction'
    mu = 'mu_l'
    rho_l = 'rho_l'
    dendrite_spacing_scaling = 1e-1
  []

  # Constant property functors (evaluated at T_solidus)
  [rho_l]
    type = ParsedFunctorMaterial
    property_name = 'rho_l'
    # ParsedFunctorMaterial currently requires at least one functor input.
    # Use enthalpy (the solved variable) as a dummy dependency to avoid a
    # temperature<->cp circular dependency.
    functor_names = 'h'
    functor_symbols = 'h'
    expression = '${rho_liquid}'
  []
  [rho_s]
    type = ParsedFunctorMaterial
    property_name = 'rho_s'
    functor_names = 'h'
    functor_symbols = 'h'
    expression = '5905.0'
  []
  [k_l]
    type = ParsedFunctorMaterial
    property_name = 'k_l'
    functor_names = 'h'
    functor_symbols = 'h'
    expression = '15.70 + 0.031*${T_solidus} + 2.97e-5 * ${T_solidus}^2'
  []
  [k_s]
    type = ParsedFunctorMaterial
    property_name = 'k_s'
    functor_names = 'h'
    functor_symbols = 'h'
    expression = '60.66 - 0.183*${T_solidus} + 6.03e-4 * ${T_solidus}^2 - 7.136e-7 * ${T_solidus}^3'
  []
  [cp_l]
    type = ParsedFunctorMaterial
    property_name = 'cp_l'
    functor_names = 'h'
    functor_symbols = 'h'
    expression = '${cp_liquid_val}'
  []
  [cp_s]
    type = ParsedFunctorMaterial
    property_name = 'cp_s'
    functor_names = 'h'
    functor_symbols = 'h'
    expression = '${cp_solid_val}'
  []
  [mu_l]
    type = ParsedFunctorMaterial
    property_name = 'mu_l'
    functor_names = 'h'
    functor_symbols = 'h'
    expression = '${mu}'
  []

  # Friction coefficient wiring
  [darcy_coeff_friction]
    type = ParsedFunctorMaterial
    property_name = 'darcy_coef_friction'
    functor_names = 'darcy_coef'
    functor_symbols = 'darcy_coef'
    expression = 'darcy_coef'
  []
  [forch_coeff_friction]
    type = ParsedFunctorMaterial
    property_name = 'forch_coef_friction'
    functor_names = 'U fch_coef'
    functor_symbols = 'U fch_coef'
    expression = 'fch_coef * U'
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  momentum_l_tol = 1e-12
  pressure_l_tol = 1e-12
  energy_l_tol = 1e-12
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9
  num_iterations = 30
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  energy_absolute_tolerance = 1e-11
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true

  start_time = 0.0
  end_time = 2000.0
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 15
    growth_factor = 1.1
    dt = 0.01
  []
  dtmax = 0.5
  num_piso_iterations = 0

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.05 0.05 0.0'
[]

[Postprocessors]
  [ave_p]
    type = ElementAverageValue
    variable = 'pressure'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_fl]
    type = ElementAverageValue
    variable = 'fl'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_T]
    type = ElementAverageValue
    variable = 'T'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ave_h]
    type = ElementAverageValue
    variable = 'h'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
