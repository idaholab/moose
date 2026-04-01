radius = 0.5
height_bottom = 0.35
height_middle = 0.30
height_top = 0.35

nr = 24
nz_bottom = 16
nz_middle = 12
nz_top = 16

mu = 2e-5
rho_ref = 1.2
beta = 3e-3
cp_fluid = 1000
k_fluid = 0.025
T_ref = 300
T_initial = 300
T_right = 300
q_middle = 2e4
gravity_y = -9.81

forch_coeff = 30
forch_coeff_radial = 30
form_factor = 30.0

advected_interp_method = 'upwind'

[Mesh]
  coord_type = 'RZ'
  rz_coord_axis = X
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${radius}'
    dy = '${height_bottom} ${height_middle} ${height_top}'
    ix = '${nr}'
    iy = '${nz_bottom} ${nz_middle} ${nz_top}'
    subdomain_id = '1
                    2
                    3'
  []
  [baffle_lower]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'baffle_lower'
  []
  [baffle_upper]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle_lower
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'baffle_upper'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_r
    v = superficial_z
    pressure = pressure

    rho = 'rho'
    porosity = 'porosity'

    p_diffusion_kernel = p_diffusion

    pressure_baffle_sidesets = 'baffle_lower baffle_upper'
    baffle_form_loss = '${form_factor} ${form_factor}'
    pressure_baffle_relaxation = 0.1

    debug_baffle = false

    use_flux_velocity_reconstruction = true
    use_reconstructed_pressure_gradient = true
    flux_velocity_reconstruction_relaxation = 1.0

    # flux_velocity_reconstruction_zero_flux_sidesets = 'left right top bottom'

    use_interpolated_density_in_bernoulli_jump = true
    use_corrected_pressure_gradient = true
  []
[]

[Variables]
  [superficial_r]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0.0
  []
  [superficial_z]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.0
  []
  [h_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${fparse cp_fluid * T_initial}
  []
[]

[LinearFVKernels]
  [r_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_r
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = superficial_r
    v = superficial_z
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = false
  []
  [z_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_z
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = superficial_r
    v = superficial_z
    momentum_component = 'y'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = false
  []
  [r_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_r
    momentum_component = 'x'
    rhie_chow_user_object = rc
    porosity = 'porosity'
    use_corrected_gradient = true
  []
  [z_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_z
    momentum_component = 'y'
    rhie_chow_user_object = rc
    porosity = 'porosity'
    use_corrected_gradient = true
  []
  [z_buoyancy]
    type = LinearFVMomentumBuoyancy
    variable = superficial_z
    rho = 'rho'
    reference_rho = ${rho_ref}
    gravity = '0 ${gravity_y} 0'
    momentum_component = 'y'
  []

  [u_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_r
    Forchheimer_name = forch
    porosity = porosity
    rho = rho
    u = superficial_r
    v = superficial_z
    momentum_component = 'x'
    block = 2
  []
  [v_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_z
    Forchheimer_name = forch
    porosity = porosity
    rho = rho
    u = superficial_r
    v = superficial_z
    momentum_component = 'y'
    block = 2
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusionJump
    variable = pressure
    diffusion_tensor = Ainv
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    debug_baffle_jump = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []

  [fluid_advection]
    type = LinearFVEnergyAdvection
    variable = h_fluid
    advected_quantity = enthalpy
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = rc
  []
  [fluid_diffusion]
    type = LinearFVDiffusion
    variable = h_fluid
    diffusion_coeff = kappa_fluid
    use_nonorthogonal_correction = false
  []
  [middle_heat_source]
    type = LinearFVSource
    variable = h_fluid
    source_density = q_middle_scaled
    block = 2
  []
[]

[LinearFVBCs]
  [axisymmetric_r]
    type = LinearFVVelocitySymmetryBC
    boundary = left
    variable = superficial_r
    u = superficial_r
    v = superficial_z
    momentum_component = x
  []
  [axisymmetric_z]
    type = LinearFVVelocitySymmetryBC
    boundary = left
    variable = superficial_z
    u = superficial_r
    v = superficial_z
    momentum_component = y
  []

  [pressure-symmetry]
    type = LinearFVPressureSymmetryBC
    boundary = 'left'
    variable = pressure
    HbyA_flux = 'HbyA' # Functor created in the RhieChowMassFlux UO
  []

  [noslip_r]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right top bottom'
    variable = superficial_r
    functor = 0.0
  []
  [noslip_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right top bottom'
    variable = superficial_z
    functor = 0.0
  []
  # [pressure_extrapolation]
  #   type = LinearFVExtrapolatedPressureBC
  #   boundary = 'right top bottom'
  #   variable = pressure
  #   use_two_term_expansion = true
  # []

  [pressure-extrapolation]
    type = LinearFVPressureFluxBC
    boundary = 'right top bottom'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
  []

  [right_T_fluid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    variable = T_fluid
    functor = ${T_right}
  []
  [right_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    variable = h_fluid
    functor = h_from_p_T
  []
  [insulated_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'left top bottom'
    variable = h_fluid
    functor = 0.0
    diffusion_coeff = kappa_fluid
  []
[]

[FunctorMaterials]
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 1.0 2 0.5 3 1.0'
  []
  [mu]
    type = ParsedFunctorMaterial
    property_name = mu
    expression = '${mu}'
  []
  [fluid_constants]
    type = GenericFunctorMaterial
    prop_names = 'cp_fluid kappa_fluid q_middle_scaled'
    prop_values = '${cp_fluid} ${fparse k_fluid / cp_fluid} ${fparse q_middle / cp_fluid}'
  []
  [fluid_enthalpy_material]
    type = LinearFVEnthalpyFunctorMaterial
    pressure = pressure
    T_fluid = T_fluid
    h = h_fluid
    h_from_p_T_functor = h_from_p_T_functor
    T_from_p_h_functor = T_from_p_h_functor
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
    functor_names = 'h_fluid'
    expression = 'h_fluid / ${cp_fluid}'
  []
  [rho]
    type = ParsedFunctorMaterial
    property_name = rho
    functor_names = 'T_fluid'
    expression = '${rho_ref} * (1.0 - ${beta} * (T_fluid - ${T_ref}))'
  []
  [forch]
    type = GenericVectorFunctorMaterial
    prop_names = forch
    prop_values = '${forch_coeff_radial} ${forch_coeff} ${forch_coeff}'
  []
[]

[AuxVariables]
  [T_fluid]
    type = MooseLinearVariableFVReal
    initial_condition = ${T_initial}
  []
  [rho_aux]
    type = MooseLinearVariableFVReal
    initial_condition = ${rho_ref}
  []
  [porosity_aux]
    type = MooseLinearVariableFVReal
    initial_condition = 1.0
  []
[]

[AuxKernels]
  [fluid_temperature]
    type = FunctorAux
    variable = T_fluid
    functor = T_from_p_h
    execute_on = 'initial nonlinear timestep_end'
  []
  [fluid_density]
    type = FunctorAux
    variable = rho_aux
    functor = rho
    execute_on = 'initial nonlinear timestep_end'
  []
  [porosity_field]
    type = FunctorAux
    variable = porosity_aux
    functor = porosity
    execute_on = 'initial nonlinear timestep_end'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  energy_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  energy_system = energy_system
  momentum_equation_relaxation = 0.2
  pressure_variable_relaxation = 0.1
  energy_equation_relaxation = 0.3
  num_iterations = 3000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '${fparse 0.5 * radius} ${fparse height_bottom + 0.5 * height_middle} 0.0'
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
