##########################################################
# Simulation of Gallium Melting Experiment
# Ref: Gau, C., & Viskanta, R. (1986). Melting and solidification of a pure metal on a vertical wall.
# Key physics: melting/solidification, convective heat transfer, natural convection
##########################################################

mu = 1.81e-3
rho_solid = 6093
rho_liquid = 6093
k_solid = 32
k_liquid = 32
cp_solid = 381.5
cp_liquid = 381.5
L = 80160
alpha_b = 1.2e-4
T_solidus = 302.93
T_liquidus = '${fparse T_solidus + 0.1}'
advected_interp_method = 'upwind'
T_cold = 301.15
T_hot = 311.15
Nx = 100
Ny = 50

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
    rho = ${rho_liquid}
    p_diffusion_kernel = p_diffusion
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[AuxVariables]
  [U]
    type = MooseVariableFVReal
  []
  [fl]
    type = MooseVariableFVReal
    initial_condition = 0.0
  []
  [density]
    type = MooseVariableFVReal
  []
  [th_cond]
    type = MooseVariableFVReal
  []
  [cp_var]
    type = MooseVariableFVReal
  []
  [darcy_coef]
    type = MooseVariableFVReal
  []
  [fch_coef]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
  [compute_fl]
    type = NSLiquidFractionAux
    variable = fl
    temperature = T
    T_liquidus = '${T_liquidus}'
    T_solidus = '${T_solidus}'
    execute_on = 'TIMESTEP_END'
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
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_cold}
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
    mu = ${mu}
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
    mu = ${mu}
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
    variable = T
    factor = ${fparse rho_liquid*cp_liquid}
  []
  [h_advection]
    type = LinearFVEnergyAdvection
    variable = T
    advected_quantity = temperature
    cp = ${cp_liquid}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [conduction]
    type = LinearFVDiffusion
    variable = T
    diffusion_coeff = ${k_liquid}
    use_nonorthogonal_correction = false
  []
  [energy_source]
    type = LinearFVPhaseChangeSource
    variable = T
    L = ${L}
    T_liquidus = ${T_liquidus}
    T_solidus = ${T_solidus}
    rho = 'rho_mixture'
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
    variable = T
    functor = '${T_hot}'
    boundary = 'left'
  []
  [cold_wall]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    functor = '${T_cold}'
    boundary = 'bottom'
  []
[]

[FunctorMaterials]
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    rho = rho_mixture
    cp = cp_mixture
    temperature = 'T'
  []
  [eff_cp]
    type = NSFVMixtureFunctorMaterial
    phase_2_names = '${cp_solid} ${k_solid} ${rho_solid}'
    phase_1_names = '${cp_liquid} ${k_liquid} ${rho_liquid}'
    prop_names = 'cp_mixture k_mixture rho_mixture'
    phase_1_fraction = fl
  []
  [mushy_zone_resistance]
    type = INSFVMushyPorousFrictionFunctorMaterial
    liquid_fraction = 'fl'
    mu = '${mu}'
    rho_l = '${rho_liquid}'
    dendrite_spacing_scaling = 1e-1
  []
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
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9
  num_iterations = 50
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
  dt = 0.1
  #num_steps = 300

  start_time = 0.0
  end_time = 200.0
  # [TimeStepper]
  #   type = IterationAdaptiveDT
  #   # Raise time step often but not by as much
  #   # There's a rough spot for convergence near 10% fluid fraction
  #   optimal_iterations = 15
  #   growth_factor = 1.1
  #   dt = 0.01
  # []
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
[]

# [VectorPostprocessors]
#   [vel_x]
#     type = ElementValueSampler
#     variable = 'vel_x fl'
#     sort_by = 'x'
#   []
# []

[Outputs]
  exodus = true
  csv = false
[]
