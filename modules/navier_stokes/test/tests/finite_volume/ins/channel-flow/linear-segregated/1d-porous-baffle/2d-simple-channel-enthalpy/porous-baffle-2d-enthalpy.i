mu = 2e-3
rho = 1000
advected_interp_method = 'upwind'
u_in = 2
forchheimer = 0
bf = '0 0'

cp_fluid = 4200
cp_solid = 500
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
    ix = '21 21 21'
    iy = '21'
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

[UserObjects]
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_baffle_sidesets = 'baffle baffle2'
    # pressure_gradient_limiter = 'baffle baffle2'
    # pressure_gradient_limiter_blend = 0.5
    baffle_form_loss = ${bf}
    velocity_form_loss = 'lower_epsilon higher_epsilon'
    pressure_baffle_relaxation = 0.1
    debug_baffle = false
    use_flux_velocity_reconstruction = true
    use_reconstructed_pressure_gradient = true
    flux_velocity_reconstruction_relaxation = 1.0
    flux_velocity_reconstruction_zero_flux_sidesets = 'top_to_1 top_to_2 top_to_3 bottom_to_1 bottom_to_2 bottom_to_3'
    use_corrected_pressure_gradient = false
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = ${u_in}
  []
  [superficial_v]
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
  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = ${T_initial}
    block = 2
  []
[]

[LinearFVKernels]
  [u_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = true
  []
  [v_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = true
  []
  [u_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_u
    momentum_component = 'x'
    rhie_chow_user_object = rc
    porosity = porosity
    use_corrected_gradient = true
  []
  [v_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    porosity = porosity
    use_corrected_gradient = true
  []
  [u_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_u
    Forchheimer_name = forch
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    block = 2
  []
  [v_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_v
    Forchheimer_name = forch
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    v = superficial_v
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
  [fluid_exchange]
    type = LinearFVEnthalpyVolumetricHeatTransfer
    variable = h_fluid
    h_solid_fluid = h_vol
    cp = cp_fluid
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = false
    block = 2
  []
  [fluid_source]
    type = LinearFVSource
    variable = h_fluid
    source_density = q_fluid_scaled
  []

  [solid_diffusion]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = ${k_solid}
    use_nonorthogonal_correction = false
    block = 2
  []
  [solid_exchange]
    type = LinearFVEnthalpyVolumetricHeatTransfer
    variable = T_solid
    h_solid_fluid = h_vol
    cp = 1.0
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = true
    block = 2
  []
  [solid_source]
    type = LinearFVSource
    variable = T_solid
    source_density = q_solid_scaled
    block = 2
  []
[]

[LinearFVBCs]
  [left_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = superficial_u
    functor = ${u_in}
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = superficial_u
    use_two_term_expansion = false
  []

  [left_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = superficial_v
    functor = 0.0
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = superficial_v
    use_two_term_expansion = false
  []

  # [noslip_v]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   boundary = 'top_to_1 top_to_3 bottom_to_1 bottom_to_3'
  #   variable = superficial_v
  #   functor = 0.0
  # []
  # [noslip_u]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   boundary = 'top_to_1 top_to_3 bottom_to_1 bottom_to_3'
  #   variable = superficial_u
  #   functor = 0.0
  # []

  [symmetry-u]
    type = LinearFVVelocitySymmetryBC
    # boundary = 'top_to_2 bottom_to_2'
    boundary = 'top_to_1 top_to_3 bottom_to_1 bottom_to_3 top_to_2 bottom_to_2'
    variable = superficial_u
    u = superficial_u
    v = superficial_v
    momentum_component = x
  []
  [symmetry-v]
    type = LinearFVVelocitySymmetryBC
    # boundary = 'top_to_2 bottom_to_2'
    boundary = 'top_to_1 top_to_3 bottom_to_1 bottom_to_3 top_to_2 bottom_to_2'
    variable = superficial_v
    u = superficial_u
    v = superficial_v
    momentum_component = y
  []

  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    variable = pressure
    functor = 0.0
  []

  [left_T_fluid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = T_fluid
    functor = ${T_inlet}
  []
  [left_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = h_fluid
    functor = h_from_p_T
  []
  [walls_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'top_to_1 top_to_2 top_to_3 bottom_to_1 bottom_to_2 bottom_to_3'
    variable = h_fluid
    functor = 0.0
    diffusion_coeff = kappa_fluid
  []
  [outlet_h_fluid]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = h_fluid
    use_two_term_expansion = false
  []

  [solid_zero_flux]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'baffle baffle2 top_to_2 bottom_to_2'
    variable = T_solid
    functor = 0.0
    diffusion_coeff = kappa_solid
  []
[]

[FunctorMaterials]
  [forch]
    type = GenericVectorFunctorMaterial
    prop_names = forch
    prop_values = '${forchheimer} ${forchheimer} ${forchheimer}'
  []
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 1.0 2 0.5 3 1.0'
  []
  [fluid_constants]
    type = GenericFunctorMaterial
    prop_names = 'cp_fluid kappa_fluid h_vol q_fluid_scaled'
    prop_values = '${cp_fluid} ${fparse k_fluid/cp_fluid} ${h_vol} 0'
  []
  [solid_constants]
    type = GenericFunctorMaterial
    prop_names = 'cp_solid kappa_solid'
    prop_values = '${cp_solid} ${fparse k_solid/cp_solid}'
  []
  [q_solid]
    type = GenericFunctorMaterial
    prop_names = 'q_solid'
    prop_values = ${q_solid}
    block = 2
  []
  [q_solid_scaled]
    type = ParsedFunctorMaterial
    property_name = q_solid_scaled
    functor_names = 'porosity q_solid'
    expression = '(1 - porosity) * q_solid'
    block = 2
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
[]

[Postprocessors]
  [p_left]
    type = SideAverageValue
    variable = pressure
    boundary = left
  []
  [p_right]
    type = SideAverageValue
    variable = pressure
    boundary = right
  []
  [p_jump]
    type = ParsedPostprocessor
    expression = 'p_left - p_right'
    pp_names = 'p_left p_right'
  []
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
    variable = h_fluid
    boundary = right
  []
[]

[AuxVariables]
  [porosity_aux]
    type = MooseLinearVariableFVReal
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    initial_condition = ${T_initial}
  []
[]

[AuxKernels]
  [por]
    type = FunctorAux
    variable = porosity_aux
    functor = porosity
    execute_on = 'NONLINEAR'
  []
  [fluid_temperature]
    type = FunctorAux
    variable = T_fluid
    functor = T_from_p_h
    execute_on = 'NONLINEAR'
  []
[]

[VectorPostprocessors]
  [u_line]
    type = LineValueSampler
    variable = superficial_u
    start_point = '0 0.25 0'
    end_point = '1.5 0.25 0'
    num_points = 401
    sort_by = id
  []
  [p_line]
    type = LineValueSampler
    variable = pressure
    start_point = '0 0.25 0'
    end_point = '1.5 0.25 0'
    num_points = 401
    sort_by = id
  []
  [T_fluid_line]
    type = LineValueSampler
    variable = T_fluid
    start_point = '0 0.25 0'
    end_point = '1.5 0.25 0'
    num_points = 401
    sort_by = id
  []
  [T_solid_line]
    type = LineValueSampler
    variable = T_solid
    start_point = '0 0.25 0'
    end_point = '1.5 0.25 0'
    num_points = 401
    sort_by = id
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
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  energy_system = energy_system
  solid_energy_system = solid_energy_system
  momentum_equation_relaxation = 0.4
  pressure_variable_relaxation = 0.1
  energy_equation_relaxation = 0.8
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
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
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = FINAL
[]
