# Fluid Properties of Water at 20 degrees Celsius
rho = 1000
mu = 1e-3
k = 0.6
cp = 4184
beta = 0.000214

### Operating Conditions
cold_temp = 293.15
hot_temp = 353.15
pressure_air = 101325 # [Pascals]
u_ref = 0.001 # [m/s]; A reference flow speed
Pr_t = 0.9
side_length = 1
# Rayleigh number = 8.6e5

### Initial Conditions
intensity = 0.01 # Turbulent intensity
k_init = '${fparse 1.5 * (intensity * u_ref) ^ 2}' # Turbulent initial kinetic energy
eps_init = '${fparse ((C_mu ^ 0.75) * (k_init ^ 1.5)) / side_length}' # Turbulent initial dissipation rate

### k-epsilon Closure Parameters: Widely accepted values from Launder and Sharma (1974)
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Wall Conditions
walls = 'left right top bottom'
bulk_wall_treatment = false
wall_treatment = 'eq_newton'

advected_interp_method = 'upwind'



[Mesh]
    [cavity]
        type = GeneratedMeshGenerator
        dim = 2
        xmin = 0
        xmax = ${side_length}
        ymin = 0
        ymax =${side_length}
        nx = 25
        ny = 25
    []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system TKE_system TKED_system'
  previous_nl_solution_required = true
[]

[Physics]
    [NavierStokes]
        [FlowSegregated]
            [flow]
                compressibility = 'incompressible'
                verbose = true

                velocity_variable = 'velocity_x velocity_y'

                density = 'rho'
                dynamic_viscosity = 'mu'

                initial_velocity = '1e-15 1e-15 0'
                initial_pressure = '${pressure_air}'

                wall_boundaries = ${walls}
                momentum_wall_types = 'noslip noslip noslip noslip'

                momentum_advection_interpolation = ${advected_interp_method}

                # Boussineq parameters
                # bous
                gravity = '0 -9.81 0'
            []
        []
        [FluidHeatTransferSegregated]
            [energy]
                coupled_flow_physics = flow
                verbose = true

                thermal_conductivity = '${k}'
                specific_heat = '${cp}'

                initial_temperature = ${cold_temp}

                energy_wall_types = 'heatflux heatflux fixed-temperature fixed-temperature'
                energy_wall_functors = '0 0 ${cold_temp} ${hot_temp}'

                energy_advection_interpolation = ${advected_interp_method}
            []
        []
    []
[]

[Variables]
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
[]

[LinearFVKernels]
    [v_buoyancy]
        type = LinearFVMomentumBoussinesq
        variable = velocity_y
        gravity = '0 -9.81 0'
        rho = ${rho}
        ref_temperature = ${cold_temp}
        momentum_component = 'y'
        T_fluid = T_fluid
        alpha_name = ${beta}
    []
  [TKE_advection]
    type = LinearFVTurbulentAdvection
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
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
    u = velocity_x
    v = velocity_y
    epsilon = TKED
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    C_pl = 10

    # The parameters below initialize the C3-buoyancy term
    temperature = T_fluid
    alpha_name = ${beta}
    Pr_t = ${Pr_t}
    gravity = '0 -9.81 0'
  []

  [TKED_advection]
    type = LinearFVTurbulentAdvection
    variable = TKED
    walls = ${walls}
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
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
    u = velocity_x
    v = velocity_y
    tke = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    C_pl = 10

    # The parameters below initialize the C3-buoyancy term
    temperature = T_fluid
    alpha_name = ${beta}
    Pr_t = ${Pr_t}
    gravity = '0 -9.81 0'
  []
  [turbulent_heat_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = T_fluid
    diffusion_coeff = 'k_t'
    use_nonorthogonal_correction = true
    walls = ${walls}
  []

  # missing a term for now
  [turbulent_momx]
    type = LinearFVTurbulentDiffusion
    variable = velocity_x
    diffusion_coeff = 'mu_t'
    use_nonorthogonal_correction = true
    walls = ${walls}
  []
  [turbulent_momy]
    type = LinearFVTurbulentDiffusion
    variable = velocity_y
    diffusion_coeff = 'mu_t'
    use_nonorthogonal_correction = true
    walls = ${walls}
  []
[]

[LinearFVBCs]
  [walls_mu_t]
    type = LinearFVTurbulentViscosityWallFunctionBC
    boundary = ${walls}
    variable = 'mu_t'
    u = velocity_x
    v = velocity_y
    rho = ${rho}
    mu = ${mu}
    tke = TKE
    wall_treatment = ${wall_treatment}
  []
[]

[AuxVariables]
  [mu_t]
    type = MooseLinearVariableFVReal
    initial_condition = '${fparse rho * C_mu * ${k_init}^2 / eps_init}'
  []
  [k_t]
    type = MooseLinearVariableFVReal
  []
  [yplus]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [compute_mu_t]
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    tke = TKE
    epsilon = TKED
    mu = ${mu}
    rho = ${rho}
    u = velocity_x
    v = velocity_y
    bulk_wall_treatment = ${bulk_wall_treatment}
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
    mu_t_ratio_max = 1e20
  []
  [compute_k_t]
    type = TurbulentConductivityAux
    cp = ${cp}
    mu_t = 'mu_t'
    variable = 'k_t'
    Pr_t = ${Pr_t}
  []
  [compute_y_plus]
    type = RANSYPlusAux
    variable = yplus
    tke = TKE
    mu = ${mu}
    rho = ${rho}
    u = velocity_x
    v = velocity_y
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
  []
[]

[FunctorMaterials]
  [constant_functors]
    type = GenericFunctorMaterial
    prop_names = 'cp alpha mu rho rho_cp'
    prop_values = '${cp} ${beta} ${mu} ${rho} ${rho}'
  []
[]

[VectorPostprocessors]
  [horizontal]
    type = LineValueSampler
    start_point = '${fparse 0.5 * side_length} 0 0'
    end_point = '${fparse 0.5 * side_length} ${side_length} 0'
    num_points = 25
    variable = 'velocity_x velocity_y pressure TKE TKED'
    sort_by = 'y'
  []
  [vertical]
    type = LineValueSampler
    start_point = '0 ${fparse 0.5 * side_length} 0'
    end_point = '${side_length} ${fparse 0.5 * side_length} 0'
    num_points = 25
    variable = 'velocity_x velocity_y pressure TKE TKED'
    sort_by = 'x'
  []
[]

[Executioner]
  type = PIMPLE
  # Try SIMPLE solver

  momentum_l_abs_tol = 1e-11
  pressure_l_abs_tol = 1e-11
  energy_l_abs_tol = 1e-11
  turbulence_l_abs_tol = 1e-11
  momentum_l_tol = 1e-8
  pressure_l_tol = 1e-8
  energy_l_tol = 1e-8
  turbulence_l_tol = 1e-8
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-8
  turbulence_absolute_tolerance = '1e-8 1e-8'

  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  turbulence_systems = 'TKE_system TKED_system'

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.4
  turbulence_equation_relaxation = '0.25 0.25'

  num_iterations = 1000
  print_fields = false
  continue_on_max_its = true
  dt = 1
  num_steps = 200
  num_piso_iterations = 0

  pin_pressure = true
  pressure_pin_value = 101325
  pressure_pin_point = '0.5 0.5 0.0'

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'

  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'

  turbulence_petsc_options_iname = '-pc_type -pc_hypre_type'
  turbulence_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
    exodus = true

    [csv]
      type = CSV
      execute_on = 'FINAL'
    []
[]