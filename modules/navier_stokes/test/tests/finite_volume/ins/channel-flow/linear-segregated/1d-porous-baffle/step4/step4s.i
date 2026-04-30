# mu = 1e-5
p_out = 5.5e6
rho_f = 8.60161

bed_radius = 1.2
bed_height = 10.0
bed_porosity = 0.39
cavity_height = 0.5
# bed_forch = 10.14     #there is a difference in convention between the SIMPLE and the Newton solver
# 52 * 0.39/2 = 10.14
bed_forch = 10.14 #9.9

cp_f = 5200
k_f = 0.25
kappa_h = '${fparse k_f / cp_f}'
k_s = 20
alpha = 2e4
pebble_diameter = 0.06

power_fn_scaling = 0.88689239556
offset = 0.56331

mass_flow_rate = 60.0   #value with low rho
flow_area = '${fparse pi * bed_radius * bed_radius}'
flow_vel = '${fparse mass_flow_rate / (flow_area * rho_f)}'

T_inlet = 300
h_inlet = '${fparse cp_f * T_inlet}'

advected_interp_method = 'upwind'

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${bed_radius}'
    ix = '6'
    dy = '${bed_height} ${cavity_height}'
    iy = '40            2'
    subdomain_id = '1 2'
  []


  [rename_blocks]
    type = RenameBlockGenerator
    old_block = '1 2'
    new_block = 'bed cavity'
    input = gen
  []

  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = rename_blocks
    primary_block = 'bed'
    paired_block = 'cavity'
    new_boundary = 'baffle'
  []
  coord_type = RZ

[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
  previous_nl_solution_required = true
[]

[FluidProperties]
  [fp]
    type = HeliumFluidProperties
  []
[]

[UserObjects]
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = 'rho_aux'
    # rho = 'rho'
    porosity = 'porosity'
    p_diffusion_kernel = p_diffusion

    pressure_baffle_sidesets = 'baffle'
    pressure_baffle_relaxation = 0.05

    debug_baffle = false

    use_flux_velocity_reconstruction = true     #diverges if false
    use_reconstructed_pressure_gradient = true
    flux_velocity_reconstruction_relaxation = 1.0
    reconstructed_pressure_gradient_feedback_relaxation = 1.0
    flux_velocity_reconstruction_zero_flux_sidesets = 'left right'
    # flux_velocity_reconstruction_zero_flux_sidesets = 'top_to_1 top_to_2 bottom_to_1 bottom_to_2'

    use_interpolated_density_in_bernoulli_jump = true
    # pressure_gradient_limiter = 'baffle'
    # pressure_gradient_limiter_blend = 1.0
    use_corrected_pressure_gradient = true
    pressure_projection_method = standard
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-16
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.05
  pressure_variable_relaxation = 0.1
  num_iterations = 12000
  pressure_absolute_tolerance = 1e-7
  # The radial residual is normalized by a near-zero exact radial velocity in this 1D RZ setup,
  # so use a practical tolerance for u_system and a strict tolerance for v_system.
  momentum_absolute_tolerance = '1e-1 1e-7'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = false

  energy_system = energy_system
  energy_l_abs_tol = 1e-12
  energy_l_tol = 0
  energy_equation_relaxation = 0.25
  energy_field_relaxation = 1.0
  energy_absolute_tolerance = 1e-7
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'

  solid_energy_system = solid_energy_system
  solid_energy_l_abs_tol = 1e-12
  solid_energy_l_tol = 0
  solid_energy_field_relaxation = 1.0
  solid_energy_absolute_tolerance = 1e-7
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0.0
  []
  [superficial_v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = -${flow_vel}
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = ${p_out}
  []

  [h_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${h_inlet}
  []

  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = ${T_inlet}
    block = 'bed'
  []


[]

[LinearFVKernels]
  [u_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
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
    mu = 'mu'
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
    porosity = 'porosity'
    use_corrected_gradient = true
  []
  [v_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    porosity = 'porosity'
    use_corrected_gradient = true
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

  [u_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_u
    Forchheimer_name = Forchheimer_coefficient
    porosity = porosity
    rho = rho_aux
    # rho = rho
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
  []

  [v_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_v
    Forchheimer_name = Forchheimer_coefficient
    porosity = porosity
    rho = rho_aux
    # rho = rho
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
  []

  [fluid_energy_advection]
    type = LinearFVEnergyAdvection
    variable = h_fluid
    advected_quantity = enthalpy
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = rc
  []

  [fluid_energy_diffusion]
    type = LinearFVDiffusion
    variable = h_fluid
    # diffusion_coeff = kappa_h_var
    diffusion_coeff = kappa_h
    use_nonorthogonal_correction = false
  []

  [fluid_solid_exchange]
    type = LinearFVEnthalpyVolumetricHeatTransfer
    variable = h_fluid
    h_solid_fluid = alpha
    cp = cp
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = false
    block = 'bed'
  []

  [solid_energy_diffusion]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = kappa_s
    use_nonorthogonal_correction = false
    block = 'bed'
  []

  [source]
    type = LinearFVSource
    variable = T_solid
    source_density = heat_source_fn
    block = 'bed'
  []

  [convection_pebble_bed_fluid]
    type = LinearFVEnthalpyVolumetricHeatTransfer
    variable = T_solid
    h_solid_fluid = alpha
    cp = 1.0
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = true
    block = 'bed'
  []
[]

[LinearFVBCs]
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = top
    variable = superficial_u
    functor = 0
  []
  [inlet_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = top
    variable = superficial_v
    functor = -${flow_vel}
  []

  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'top'
    variable = pressure
    use_two_term_expansion = true
  []

  # Fix the outlet pressure and leave the outlet velocity free.
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = bottom
    variable = superficial_u
    use_two_term_expansion = true
    assume_fully_developed_flow = true
    # assume_fully_developed_flow = true
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = bottom
    variable = superficial_v
    use_two_term_expansion = true
    assume_fully_developed_flow = true
    # assume_fully_developed_flow = true
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = bottom
    variable = pressure
    functor = ${p_out}
  []

  # Symmetry removes any wall losses and keeps the exact solution one-dimensional.
  [symmetry_u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'left right'
    variable = superficial_u
    u = superficial_u
    v = superficial_v
    momentum_component = x
  []
  [symmetry_v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'left right'
    variable = superficial_v
    u = superficial_u
    v = superficial_v
    momentum_component = y
  []
  [pressure_symmetric]
    type = LinearFVPressureSymmetryBC
    boundary = 'left right'
    variable = pressure
    HbyA_flux = 'HbyA'
  []


  [top_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = top
    variable = h_fluid
    # functor = h_from_p_T
    functor = ${h_inlet}
  []

  [side_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'left right'
    variable = h_fluid
    functor = 0.0
    # diffusion_coeff = kappa_h_var
    diffusion_coeff = kappa_h
  []

  [bottom_h_fluid]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = bottom
    variable = h_fluid
    use_two_term_expansion = false
  []

  [side_T_solid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'left right'
    variable = T_solid
    functor = 0.0
    diffusion_coeff = kappa_s
  []

[]

[Functions]
  [heat_source_fn]
    type = ParsedFunction
    expression = '${power_fn_scaling} * (-1.0612e4 * pow(y+${offset}, 4) + 1.5963e5 * pow(y+${offset}, 3)
                   -6.2993e5 * pow(y+${offset}, 2) + 1.4199e6 * (y+${offset}) + 5.5402e4)'
  []

  # [rho_parsed]
  #   type = ParsedFunction
  #   expression = 'if(y < 10, 2.65 + 0.11*exp(0.40*y), 8.60161)'
  # []

  # [T_solid_initial]
  #   type = ParsedFunction
  #   expression = 'if(y < 5.5,
  #                    960 + 12.7272727*y,
  #                    1030 - 144.4444444*(y-5.5))'
  # []

  # [T_fluid_initial]
  #   type = ParsedFunction
  #   expression = '920 - 59.047619*y'
  # []

  # [h_fluid_initial]
  #   type = ParsedFunction
  #   expression = '4.8e6 - 314285.7143*y'
  # []
[]

# [FVICs]
#   [T_solid_ic]
#     type = FVFunctionIC
#     variable = T_solid
#     function = T_solid_initial
#     block = 'bed'
#   []

#   [T_fluid_ic]
#     type = FVFunctionIC
#     variable = T_fluid
#     function = T_fluid_initial
#   []

#   [h_fluid_ic]
#     type = FVFunctionIC
#     variable = h_fluid
#     function = h_fluid_initial
#   []
# []

[FunctorMaterials]

  [fluid_props]
    type = GeneralFunctorFluidProps
    fp = fp
    pressure = pressure
    # T_fluid = ${T_inlet}
    # Use the enthalpy-temperature functor directly for properties so density is not one
    # nonlinear iteration behind the energy solve through the T_fluid aux variable.
    T_fluid = T_from_p_h
    speed = 1
    porosity = porosity
    characteristic_length = ${pebble_diameter}
  []


  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = 'bed ${bed_porosity} cavity 1'
  []

  [drag_bed]
    type = GenericVectorFunctorMaterial
    prop_names = 'bed_forch_vec'
    prop_values = '${bed_forch} ${bed_forch} ${bed_forch}'
  []

  [drag_cavity]
    type = GenericVectorFunctorMaterial
    prop_names = 'cavity_forch_vec'
    prop_values = '0 0 0'
  []

  [forch]
    type = PiecewiseByBlockVectorFunctorMaterial
    prop_name = 'Forchheimer_coefficient'
    subdomain_to_prop_value = 'bed bed_forch_vec
                              cavity cavity_forch_vec'
  []


  [fluid_enthalpy_material]
    type = LinearFVEnthalpyFunctorMaterial
    pressure = pressure
    T_fluid = T_fluid
    h = h_fluid
    fp = fp
  []

  [fluid_constants]
    type = GenericFunctorMaterial
    prop_names = 'cp_f kappa_h'
    prop_values = '${cp_f} ${kappa_h}'
  []

  [kappa_h_var]
    type = ADParsedFunctorMaterial
    property_name = kappa_h_var
    functor_names = 'k cp'
    expression = 'k / cp'
    enable_jit = false
  []

  [solid_k]
    type = GenericFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '${k_s}'
    block = 'bed'
  []

  [alpha_mat]
    type = GenericFunctorMaterial
    prop_names = 'alpha'
    prop_values = '${alpha}'
    block = 'bed'
  []
[]

[AuxVariables]
  [rho_aux]
    type = MooseLinearVariableFVReal
  []

  [rho_aux_old]
    type = MooseLinearVariableFVReal
  []

  [rho_raw_aux]
    type = MooseLinearVariableFVReal
  []

  [porosity_aux]
    type = MooseLinearVariableFVReal
  []

  [T_fluid]
    type = MooseLinearVariableFVReal
    initial_condition = ${T_inlet}
  []
[]

[AuxKernels]
  [assign_rho_aux]
    type = FunctorAux
    variable = rho_aux
    # functor = 'rho_parsed'
    functor = 'rho'
    # execute_on = 'INITIAL TIMESTEP_END'
    execute_on = 'INITIAL NONLINEAR'
  []

  # [assign_rho_raw_aux]
  #   type = FunctorAux
  #   variable = rho_raw_aux
  #   functor = rho
  #   execute_on = 'INITIAL NONLINEAR TIMESTEP_END'
  # []

  # [assign_rho_aux_old]
  #   type = FunctorAux
  #   variable = rho_aux_old
  #   functor = rho_aux
  #   execute_on = 'INITIAL NONLINEAR TIMESTEP_END'
  # []

  # [assign_rho_aux]
  #   type = RelaxedDensityAux
  #   variable = rho_aux
  #   rho_raw_aux = rho_raw_aux
  #   rho_aux_old = rho_aux_old
  #   relaxation = 0.7
  #   initialize_from_rho = true
  #   execute_on = 'INITIAL NONLINEAR TIMESTEP_END'
  # []


  [assign_porosity_aux]
    type = FunctorAux
    variable = porosity_aux
    functor = 'porosity'
    execute_on = 'initial timestep_end'
  []

  [fluid_temperature]
    type = FunctorAux
    variable = T_fluid
    functor = T_from_p_h
    execute_on = 'INITIAL NONLINEAR'
  []
[]

[Postprocessors]
  [inlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = top
    outputs = none
  []

  [outlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = bottom
    outputs = none
  []

  [pressure_drop]
    type = ParsedPostprocessor
    pp_names = 'inlet_pressure outlet_pressure'
    expression = 'inlet_pressure - outlet_pressure'
  []

  [desired_mfr]
    type = Receiver
    default = ${mass_flow_rate}
  []

  [inlet_mfr]
    type = VolumetricFlowRate
    advected_quantity = rho_aux
    # advected_quantity = rho
    vel_x = superficial_u
    vel_y = superficial_v
    boundary = top
    rhie_chow_user_object = rc
  []
  [outlet_mfr]
    type = VolumetricFlowRate
    advected_quantity = rho_aux
    # advected_quantity = rho
    vel_x = superficial_u
    vel_y = superficial_v
    boundary = bottom
    rhie_chow_user_object = rc
  []

  [u_min]
    type = ElementExtremeValue
    variable = superficial_u
    value_type = min
  []
  [u_max]
    type = ElementExtremeValue
    variable = superficial_u
    value_type = max
  []

  [v_min]
    type = ElementExtremeValue
    variable = superficial_v
    value_type = min
  []
  [v_max]
    type = ElementExtremeValue
    variable = superficial_v
    value_type = max
  []

  [top_v_avg]
    type = SideAverageValue
    variable = superficial_v
    boundary = top
  []
  [bottom_v_avg]
    type = SideAverageValue
    variable = superficial_v
    boundary = bottom
  []

  [enthalpy_inlet]
    type = VolumetricFlowRate
    advected_quantity = h_fluid
    vel_x = superficial_u
    vel_y = superficial_v
    boundary = top
    rhie_chow_user_object = rc
  []

  [enthalpy_outlet]
    type = VolumetricFlowRate
    advected_quantity = h_fluid
    vel_x = superficial_u
    vel_y = superficial_v
    boundary = bottom
    rhie_chow_user_object = rc
    advected_interp_method = upwind
  []

  [heat_source_integral]
    type = ElementIntegralFunctorPostprocessor
    functor = heat_source_fn
    block = bed
  []

  [T_solid_max]
    type = ElementExtremeValue
    variable = T_solid
    value_type = max
    block = bed
  []

  [T_outlet_avg]
    type = SideAverageValue
    variable = T_fluid
    boundary = bottom
    execute_on='INITIAL LINEAR TIMESTEP_END'
  []
[]




[Outputs]
  exodus = true
  # csv = true
  console = true
  print_linear_residuals = true
  execute_on = 'INITIAL TIMESTEP_END'
[]
