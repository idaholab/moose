fan_normal_velocity = 10.719816 # m/s
ilet_width = 0.5472
ilet_length = 0.4064
ilet_area = '${fparse ilet_width * ilet_length}'
volumetric_flow_rate = 0.31478894915 # m3/s
velocity_inlet_magnitude = '${fparse volumetric_flow_rate / ilet_area}'
# Make this negative so that it's the opposite direction of the normal vector
velocity_diri_condition = '${fparse -velocity_inlet_magnitude}'

cylinder_inner_radius = '${fparse 23.75 / 2}'
cylinder_height = 14.40
boundary_layer_cell_size = 0.25 # meters

# air
rho = 1.177
mu = 1.846e-5
k = .0262
cp = 1006
beta = 3.33e-3
alpha = '${fparse k / (cp * rho)}'
nu = '${fparse mu / rho}'

# Dimensionless numbers
L = 10
Re = '${fparse L * velocity_inlet_magnitude / nu}'
Pr = '${fparse cp * mu / k}'

# mixed laminar-turbulent forced convection flat plate correlation
Re_crit = 5e5
Nu = '${fparse (.037 * Re^(4/5) - .664 * Re_crit^(1/2)) * Pr^(1/3)}'
h = '${fparse Nu * k / L}'

T_0 = 295.9
T_hot = 373
T_cold = ${T_0}
initial_dt = .4

# turbulence parameters
### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09
walls = 'air_box_boundary air_wall_boundary air_floor_boundary air_ahu_boundary'
wall_treatment = 'neq' # Options: eq_newton, eq_incremental, eq_linearized, neq
Pr_t = 0.9

intensity = 0.01
k_init = '${fparse 1.5*(intensity * velocity_inlet_magnitude)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / ilet_width}'

# Heat from flow
floor_heat = 3.12e3 # W

# AHU parameters
# Combined volumetric flow rate (from the two units)
ahu_vfr = 23.5973725 # m3/s
ahu_cooling = ${fparse 2*275 * 10^3} # Watts
ahu_heating = ${fparse 2*450 * 10^3} # Watts

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = zach-mesh_in.e
  []
  [removal]
    type = BlockDeletionGenerator
    input = file
    block = '2 20'
  []
  [fan1_outlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = removal
    new_boundary = fan1_outlet
    paired_block = 1
    primary_block = 10
    normal = '0 0 1'
  []
  [fan2_outlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = fan1_outlet
    new_boundary = fan2_outlet
    paired_block = 1
    primary_block = 11
    normal = '0 0 1'
  []
  [fan3_outlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = fan2_outlet
    new_boundary = fan3_outlet
    paired_block = 1
    primary_block = 12
    normal = '0 0 1'
  []
  uniform_refine = 0
[]

[Problem]
  linear_sys_names = 'u_system v_system w_system pressure_system TKE_system TKED_system energy_system'
  previous_nl_solution_required = true
[]

[AuxVariables]
  [vel_mag]
    family = MONOMIAL
    order = CONSTANT
  []
  [mu_t]
    type = MooseLinearVariableFVReal
    initial_condition = '${fparse rho * C_mu * ${k_init}^2 / eps_init}'
  []
  [k_t]
    type = MooseLinearVariableFVReal
    initial_condition = 1.
  []
  [yplus]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [vel_mag]
    type = VectorMagnitudeAux
    variable = vel_mag
    x = vel_x
    y = vel_y
    z = vel_z
    execute_on = 'TIMESTEP_END'
  []
  [compute_mu_t]
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    tke = TKE
    epsilon = TKED
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    w = vel_z
    bulk_wall_treatment = false
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
    mu_t_ratio_max = 1e20
  []
  [compute_y_plus]
    type = RANSYPlusAux
    variable = yplus
    tke = TKE
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    w = vel_z
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
  []
  [compute_kt]
    type = TurbulentConductivityAux
    variable = k_t
    Pr_t = ${Pr_t}
    cp = ${cp}
    mu_t = 'mu_t'
    execute_on = 'NONLINEAR'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    body_force_kernel_names = "; ; w_buoyancy w_fan"
    pressure_projection_method = consistent
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = v_system
  []
  [vel_z]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = w_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0
  []
  [TKE]
    type = MooseLinearVariableFVReal
    solver_sys = TKE_system
    initial_condition = ${k_init}
  []
  [TKED]
    type = MooseLinearVariableFVReal
    solver_sys = TKED_system
    initial_condition = ${eps_init}
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_0}
  []
[]

[LinearFVKernels]
  [u_time]
    type = LinearFVTimeDerivative
    variable = vel_x
    factor = ${rho}
  []
  [v_time]
    type = LinearFVTimeDerivative
    variable = vel_y
    factor = ${rho}
  []
  [w_time]
    type = LinearFVTimeDerivative
    variable = vel_z
    factor = ${rho}
  []
  [u_advection_stress_turbulent]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    mu = 'mu_t'
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'x'
    use_nonorthogonal_correction = true
  []
  [u_diffusion]
    type = LinearFVDiffusion
    variable = vel_x
    diffusion_coeff = ${mu}
    use_nonorthogonal_correction = true
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    mu = 'mu_t'
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'y'
    use_nonorthogonal_correction = true
  []
  [v_diffusion]
    type = LinearFVDiffusion
    variable = vel_y
    diffusion_coeff = ${mu}
    use_nonorthogonal_correction = true
  []
  [w_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_z
    mu = 'mu_t'
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'z'
    use_nonorthogonal_correction = true
  []
  [w_diffusion]
    type = LinearFVDiffusion
    variable = vel_z
    diffusion_coeff = ${mu}
    use_nonorthogonal_correction = true
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
  [w_pressure]
    type = LinearFVMomentumPressure
    variable = vel_z
    pressure = pressure
    momentum_component = 'z'
  []
  [w_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_z
    T_fluid = T_fluid
    gravity = '0 0 -9.8'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${beta}
    momentum_component = 'z'
  []
  [w_fan]
    type = LinearFVSource
    variable = vel_z
    source_density = 'fan_source'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []

  [TKE_time]
    type = LinearFVTimeDerivative
    variable = TKE
    factor = ${rho}
  []
  [TKE_advection]
    type = LinearFVTurbulentAdvection
    variable = TKE
  []
  [TKE_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKE
    diffusion_coeff = ${mu}
    use_nonorthogonal_correction = true
  []
  [TKE_turb_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKE
    diffusion_coeff = 'mu_t'
    scaling_coeff = ${sigma_k}
    use_nonorthogonal_correction = true
  []
  [TKE_source_sink]
    type = LinearFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    w = vel_z
    epsilon = TKED
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    C_pl = 1.0
  []

  [TKED_time]
    type = LinearFVTimeDerivative
    variable = TKED
    factor = ${rho}
  []
  [TKED_advection]
    type = LinearFVTurbulentAdvection
    variable = TKED
    walls = ${walls}
  []
  [TKED_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKED
    diffusion_coeff = ${mu}
    use_nonorthogonal_correction = true
    walls = ${walls}
  []
  [TKED_turb_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKED
    diffusion_coeff = 'mu_t'
    scaling_coeff = ${sigma_eps}
    use_nonorthogonal_correction = true
    walls = ${walls}
  []
  [TKED_source_sink]
    type = LinearFVTKEDSourceSink
    variable = TKED
    u = vel_x
    v = vel_y
    w = vel_z
    tke = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    C_pl = 1.0
  []

  [heat_time]
    type = LinearFVTimeDerivative
    variable = T_fluid
    factor = '${fparse rho*cp}'
  []
  [heat_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = temperature
    cp = ${cp}
  []
  [heat_conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = k_tot
    use_nonorthogonal_correction = true
  []
[]

[Functions]
  [ahu_on_function]
    type = PostprocessorFunction
    pp = ahu_on
  []
[]

[FunctorMaterials]
  [k_total]
    type = ParsedFunctorMaterial
    expression = '${k} + k_t'
    functor_names = 'k_t'
    property_name = 'k_tot'
  []
  [momentum_source_fan1]
    type = ParsedFunctorMaterial
    expression = '${rho} * acceleration_fan1'
    property_name = 'fan_source'
    functor_names = 'acceleration_fan1'
    block = '10'
  []
  [momentum_source_fan2]
    type = ParsedFunctorMaterial
    expression = '${rho} * acceleration_fan2'
    property_name = 'fan_source'
    functor_names = 'acceleration_fan2'
    block = '11'
  []
  [momentum_source_fan3]
    type = ParsedFunctorMaterial
    expression = '${rho} * acceleration_fan3'
    property_name = 'fan_source'
    functor_names = 'acceleration_fan3'
    block = '12'
  []
  [momentum_source_fan_null]
    type = GenericFunctorMaterial
    prop_names = 'fan_source'
    prop_values = '0'
    block = '1'
  []
  [h]
    type = ParsedFunctorMaterial
    expression = '${rho} * ${cp} * T_fluid'
    functor_names = 'T_fluid'
    property_name = 'h'
  []
[]

[LinearFVBCs]
  [inlet_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet_1 inlet_2 inlet_3'
    functor = ${velocity_diri_condition}
    variable = vel_x
    normal_component = 'x'
  []
  [inlet_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet_1 inlet_2 inlet_3'
    functor = ${velocity_diri_condition}
    variable = vel_y
    normal_component = 'y'
  []
  [inlet_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet_1 inlet_2 inlet_3'
    functor = ${velocity_diri_condition}
    variable = vel_z
    normal_component = 'z'
  []
  [inlet_TKE]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet_1 inlet_2 inlet_3'
    functor = '${k_init}'
    variable = TKE
  []
  [inlet_TKED]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet_1 inlet_2 inlet_3'
    functor = '${eps_init}'
    variable = TKED
  []
  [inlet_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet_1 inlet_2 inlet_3'
    functor = ${T_cold}
    variable = T_fluid
  []

  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'outlet_1 outlet_2'
    variable = pressure
    functor = 0
  []
  [outlet_x]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = true
    boundary = 'outlet_1 outlet_2'
  []
  [outlet_y]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = true
    boundary = 'outlet_1 outlet_2'
  []
  [outlet_z]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_z
    use_two_term_expansion = true
    boundary = 'outlet_1 outlet_2'
  []
  [outlet_TKE]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = TKE
    use_two_term_expansion = true
    boundary = 'outlet_1 outlet_2'
  []
  [outlet_TKED]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = TKED
    use_two_term_expansion = true
    boundary = 'outlet_1 outlet_2'
  []
  [outlet_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T_fluid
    use_two_term_expansion = true
    boundary = 'outlet_1 outlet_2'
  []

  [no_slip_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = ${walls}
    functor = 0
  []
  [no_slip_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = ${walls}
    functor = 0
  []
  [no_slip_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_z
    boundary = ${walls}
    functor = 0
  []

  [walls_mu_t]
    type = LinearFVTurbulentViscosityWallFunctionBC
    boundary = ${walls}
    variable = 'mu_t'
    u = vel_x
    v = vel_y
    w = vel_z
    rho = ${rho}
    mu = ${mu}
    tke = TKE
    wall_treatment = ${wall_treatment}
  []

  [pressure-flux]
    type = LinearFVPressureFluxBC
    boundary = 'air_box_boundary air_wall_boundary air_floor_boundary air_ahu_boundary inlet_1 inlet_2 inlet_3 ahu_intake ahu_exhaust'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
  []

  [T_floor]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'air_floor_boundary'
    functor = floor_heat_density
    variable = T_fluid
  []
  [T_hot]
    type = LinearFVConvectiveHeatTransferBC
    boundary = 'air_box_boundary'
    variable = T_fluid
    h = ${h}
    T_fluid = T_fluid
    T_solid = ${T_hot}
  []
  [T_cold]
    type = LinearFVConvectiveHeatTransferBC
    boundary = 'air_wall_boundary'
    T_solid = ${T_cold}
    T_fluid = T_fluid
    variable = T_fluid
    h = ${h}
  []

  # ahu off
  [no_slip_x_ahu]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'ahu_intake ahu_exhaust'
    functor = 0
    enable = true
  []
  [no_slip_y_ahu]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'ahu_intake ahu_exhaust'
    functor = 0
    enable = true
  []
  [no_slip_z_ahu]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_z
    boundary = 'ahu_intake ahu_exhaust'
    functor = 0
    enable = true
  []
  # ahu on. Note that this is outflow from the simulation boundary so sign should be positive
  [ahu_inlet_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_intake'
    functor = 'ahu_inlet_velocity'
    variable = vel_x
    normal_component = 'x'
    enable = false
  []
  [ahu_inlet_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_intake'
    functor = 'ahu_inlet_velocity'
    variable = vel_y
    normal_component = 'y'
    enable = false
  []
  [ahu_inlet_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_intake'
    functor = 'ahu_inlet_velocity'
    variable = vel_z
    normal_component = 'z'
    enable = false
  []
  [ahu_inlet_TKE]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = TKE
    use_two_term_expansion = true
    boundary = 'ahu_intake'
    enable = false
  []
  [ahu_inlet_TKED]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = TKED
    use_two_term_expansion = true
    boundary = 'ahu_intake'
    enable = false
  []
  [ahu_inlet_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T_fluid
    use_two_term_expansion = true
    boundary = 'ahu_intake'
    enable = false
  []
  [ahu_exhaust_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_exhaust'
    functor = 'ahu_exhaust_velocity'
    variable = vel_x
    normal_component = 'x'
    enable = false
  []
  [ahu_exhaust_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_exhaust'
    functor = 'ahu_exhaust_velocity'
    variable = vel_y
    normal_component = 'y'
    enable = false
  []
  [ahu_exhaust_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_exhaust'
    functor = 'ahu_exhaust_velocity'
    variable = vel_z
    normal_component = 'z'
    enable = false
  []
  [ahu_exhaust_TKE]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_exhaust'
    functor = '${k_init}'
    variable = TKE
  []
  [ahu_exhaust_TKED]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_exhaust'
    functor = '${eps_init}'
    variable = TKED
  []
  [ahu_exhaust_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'ahu_exhaust'
    functor = 'ahu_T_out'
    variable = T_fluid
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-8
  pressure_l_abs_tol = 1e-8
  turbulence_l_abs_tol = 1e-8
  energy_l_abs_tol = 1e-8
  momentum_l_tol = 0
  pressure_l_tol = 0
  turbulence_l_tol = 0
  energy_l_tol = 0
  momentum_systems = 'u_system v_system w_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  turbulence_systems = 'TKE_system TKED_system'
  momentum_equation_relaxation = 0.80
  pressure_variable_relaxation = 0.95
  turbulence_equation_relaxation = '0.9 0.9'
  energy_equation_relaxation = 0.9
  num_iterations = 50
  pressure_absolute_tolerance = 1e-4
  momentum_absolute_tolerance = 1e-4
  turbulence_absolute_tolerance = '1e-6 1e-6'
  energy_absolute_tolerance = 1e-4
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  momentum_petsc_options_value = 'hypre boomeramg 4 1 0.1 0.6 HMIS ext+i'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  pressure_petsc_options_value = 'hypre boomeramg 2 1 0.1 0.6 HMIS ext+i'
  turbulence_petsc_options_iname = '-pc_type -pc_hypre_type'
  turbulence_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  energy_petsc_options_value = 'hypre boomeramg 4 1 0.1 0.6 HMIS ext+i'
  print_fields = false

  num_steps = 5000
  num_piso_iterations = 0

  [TimeStepper]
    type = PostprocessorDT
    postprocessor = new_dt_for_unity_cfl
    dt = ${initial_dt}
    scale = 1
  []

  scheme = bdf2
[]

[Outputs]
  [nemesis]
    type = Nemesis
    time_step_interval = 1
  []
  [console]
    type = Console
    hide = 'floor_area floor_heat_density avg_elem_size cfl new_dt_for_unity_cfl rayleigh t_diff dt area_fan1 area_fan2 area_fan3'
  []
  csv = true
  checkpoint = true
  perf_graph = true
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]

[Postprocessors]
  [rayleigh]
    type = RayleighNumber
    cp_ave = ${cp}
    gravity_magnitude = 9.8
    k_ave = ${k}
    l = ${L}
    mu_ave = ${mu}
    rho_ave = ${rho}
    beta = ${beta}
    T_cold = ${T_cold}
    T_hot = ${T_hot}
    execute_on = 'initial timestep_end'
  []
  [vel_max]
    type = ElementExtremeValue
    variable = vel_mag
  []
  [vel_avg]
    type = ElementAverageValue
    variable = vel_mag
  []
  [T_max]
    type = ElementExtremeValue
    variable = T_fluid
    value_type = max
  []
  [T_avg]
    type = ElementAverageValue
    variable = T_fluid
  []
  [T_boundary_max]
    type = SideExtremeValue
    variable = T_fluid
    value_type = max
    boundary = 'air_wall_boundary'
  []
  [dt]
    type = TimestepSize
  []
  [cfl]
    type = ParsedPostprocessor
    expression = 'vel_max * dt / h'
    constant_names = 'h'
    # Estimate by cbrt
    constant_expressions = '.56'
    pp_names = 'vel_max dt'
  []
  [new_dt_for_unity_cfl]
    type = ParsedPostprocessor
    expression = '1.5 * dt / cfl'
    pp_names = 'dt cfl'
  []
  [avg_elem_size]
    type = AverageElementSize
    execute_on = 'initial'
  []
  [t_diff]
    type = ParsedPostprocessor
    expression = 'avg_elem_size^2 / alpha'
    pp_names = 'avg_elem_size'
    constant_names = 'alpha'
    constant_expressions = '${alpha}'
    execute_on = 'initial'
  []
  [floor_area]
    type = SideIntegralFunctorPostprocessor
    boundary = air_floor_boundary
    functor = 1
    execute_on = 'initial'
  []
  [floor_heat_density]
    type = ParsedPostprocessor
    expression = 'heat / area'
    constant_names = 'heat'
    constant_expressions = ${floor_heat}
    pp_names = 'floor_area'
    pp_symbols = 'area'
    execute_on = 'initial'
  []
  [sensor_one]
    type = PointValue
    variable = T_fluid
    point = '0 ${fparse cylinder_inner_radius - boundary_layer_cell_size/2} 0.9144'
  []
  [sensor_two]
    type = PointValue
    variable = T_fluid
    point = '0 ${fparse cylinder_inner_radius - boundary_layer_cell_size/2} 10.668'
  []
  [sensor_three]
    type = PointValue
    variable = T_fluid
    point = '0 ${fparse sqrt(cylinder_inner_radius^2 - (25.60 - cylinder_height)^2) - boundary_layer_cell_size/2} 25.60'
  []
  [heating_mode]
    type = ParsedPostprocessor
    expression = 'if(((sensor_one + sensor_two + sensor_three)/3) < 293.15, 1, 0)'
    pp_names = 'sensor_one sensor_two sensor_three'
  []
  [cooling_mode]
    type = ParsedPostprocessor
    expression = 'if(((sensor_one + sensor_two + sensor_three)/3) > 298.7, 1, 0)'
    pp_names = 'sensor_one sensor_two sensor_three'
  []
  [ahu_on]
    type = ParsedPostprocessor
    expression = 'if(heating_mode|cooling_mode, 1, 0)'
    pp_names = 'heating_mode cooling_mode'
  []
  [vfr_fan1]
    type = VolumetricFlowRate
    boundary = fan1_outlet
    vel_x = vel_x
    vel_y = vel_y
    vel_z = vel_z
    advected_quantity = 1
  []
  [vfr_fan2]
    type = VolumetricFlowRate
    boundary = fan2_outlet
    vel_x = vel_x
    vel_y = vel_y
    vel_z = vel_z
    advected_quantity = 1
  []
  [vfr_fan3]
    type = VolumetricFlowRate
    boundary = fan3_outlet
    vel_x = vel_x
    vel_y = vel_y
    vel_z = vel_z
    advected_quantity = 1
  []
  [area_fan1]
    type = AreaPostprocessor
    boundary = fan1_outlet
    execute_on = 'initial'
  []
  [area_fan2]
    type = AreaPostprocessor
    boundary = fan2_outlet
    execute_on = 'initial'
  []
  [area_fan3]
    type = AreaPostprocessor
    boundary = fan3_outlet
    execute_on = 'initial'
  []
  [axial_velocity_fan1]
    type = ParsedPostprocessor
    expression = 'vfr_fan1 / area_fan1'
    pp_names = 'vfr_fan1 area_fan1'
  []
  [axial_velocity_fan2]
    type = ParsedPostprocessor
    expression = 'vfr_fan2 / area_fan2'
    pp_names = 'vfr_fan2 area_fan2'
  []
  [axial_velocity_fan3]
    type = ParsedPostprocessor
    expression = 'vfr_fan3 / area_fan3'
    pp_names = 'vfr_fan3 area_fan3'
  []
  [acceleration_fan1]
    type = ConstantPostprocessor
  []
  [acceleration_fan2]
    type = ConstantPostprocessor
  []
  [acceleration_fan3]
    type = ConstantPostprocessor
  []
  [area_ahu_inlet]
    type = AreaPostprocessor
    boundary = ahu_intake
    execute_on = 'initial'
  []
  [area_ahu_exhaust]
    type = AreaPostprocessor
    boundary = ahu_exhaust
    execute_on = 'initial'
  []
  [ahu_inlet_velocity]
    type = ParsedPostprocessor
    expression = '${ahu_vfr} / area_ahu_inlet'
    pp_names = 'area_ahu_inlet'
    execute_on = 'initial'
  []
  [ahu_exhaust_velocity]
    type = ParsedPostprocessor
    expression = '-${ahu_vfr} / area_ahu_exhaust' # negative sign to denote direction with respect to boundary normal
    pp_names = 'area_ahu_exhaust'
    execute_on = 'initial'
  []
  [ahu_heat_in]
    type = VolumetricFlowRate
    boundary = ahu_intake
    advected_quantity = h
    vel_x = vel_x
    vel_y = vel_y
    vel_z = vel_z
  []
  [ahu_heat_out_if_heated]
    type = ParsedPostprocessor
    expression = 'ahu_heat_in + ${ahu_heating}'
    pp_names = 'ahu_heat_in'
  []
  [ahu_T_out_if_heated]
    type = ParsedPostprocessor
    expression = 'ahu_heat_out_if_heated / (${ahu_vfr} * ${rho} * ${cp})'
    pp_names = 'ahu_heat_out_if_heated'
  []
  [ahu_heat_out_if_cooled]
    type = ParsedPostprocessor
    expression = 'ahu_heat_in - ${ahu_cooling}'
    pp_names = 'ahu_heat_in'
  []
  [ahu_T_out_if_cooled]
    type = ParsedPostprocessor
    expression = 'ahu_heat_out_if_cooled / (${ahu_vfr} * ${rho} * ${cp})'
    pp_names = 'ahu_heat_out_if_cooled'
  []
  [ahu_T_out]
    type = ParsedPostprocessor
    expression = 'if(heating_mode, ahu_T_out_if_heated, if(cooling_mode, ahu_T_out_if_cooled, ${T_cold}))'
    pp_names = 'heating_mode cooling_mode ahu_T_out_if_heated ahu_T_out_if_cooled'
  []
[]

[Controls]
  [fan1]
    type = PIDTransientControl
    postprocessor = axial_velocity_fan1
    target = ${fan_normal_velocity}
    parameter = 'Postprocessors/acceleration_fan1/value'
    K_integral = 0
    K_proportional = -10
    K_derivative = 0
    execute_on = 'initial timestep_begin'
  []
  [fan2]
    type = PIDTransientControl
    postprocessor = axial_velocity_fan2
    target = ${fan_normal_velocity}
    parameter = 'Postprocessors/acceleration_fan2/value'
    K_integral = 0
    K_proportional = -10
    K_derivative = 0
    execute_on = 'initial timestep_begin'
  []
  [fan3]
    type = PIDTransientControl
    postprocessor = axial_velocity_fan3
    target = ${fan_normal_velocity}
    parameter = 'Postprocessors/acceleration_fan3/value'
    K_integral = 0
    K_proportional = -10
    K_derivative = 0
    execute_on = 'initial timestep_begin'
  []
  [ahu]
    type = ConditionalFunctionEnableControl
    conditional_function = ahu_on_function
    enable_objects = 'LinearFVBCs::ahu_inlet_x LinearFVBCs::ahu_inlet_y LinearFVBCs::ahu_inlet_z LinearFVBCs::ahu_inlet_TKE LinearFVBCs::ahu_inlet_TKED LinearFVBCs::ahu_inlet_T LinearFVBCs::ahu_exhaust_x LinearFVBCs::ahu_exhaust_y LinearFVBCs::ahu_exhaust_z LinearFVBCs::ahu_exhaust_TKE LinearFVBCs::ahu_exhaust_TKED LinearFVBCs::ahu_exhaust_T'
    disable_objects = 'LinearFVBCs::no_slip_x_ahu LinearFVBCs::no_slip_y_ahu LinearFVBCs::no_slip_z_ahu'
  []
[]
