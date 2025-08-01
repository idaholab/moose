##########################################################
# Lid-driven cavity test
# Reynolds: 5,000
# Author: Dr. Mauricio Tano
# Last Update: November, 2023
# Turbulent model using:
# k-epsilon model
# Standard wall functions with temperature wall functions
# SIMPLE Solve
##########################################################

### Thermophysical Properties ###
mu = 2e-5
rho = 1.0
k = 0.01
cp = 10.0
Pr_t = 0.9

### Operation Conditions ###
lid_velocity = 1.0
side_length = 0.1

### Initial Conditions ###
intensity = 0.01
k_init = '${fparse 1.5*(intensity * lid_velocity)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / side_length}'

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Modeling parameters ###
bulk_wall_treatment = false
walls = 'left top right bottom'
wall_treatment_v = 'eq_newton' # Options: eq_newton, eq_incremental, eq_linearized, neq
wall_treatment_T = 'eq_linearized' # Options: eq_newton, eq_incremental, eq_linearized, neq

pressure_tag = "pressure_grad"

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'rc'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${side_length}
    ymin = 0
    ymax = ${side_length}
    nx = 12
    ny = 12
  []
  # Prevent test diffing on distributed parallel element numbering
  allow_renumbering = false
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system energy_system TKE_system TKED_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    solver_sys = pressure_system
    initial_condition = 0.2
    two_term_boundary_expansion = false
  []
  [T_fluid]
    type = INSFVEnergyVariable
    solver_sys = energy_system
    initial_condition = 1.0
    two_term_boundary_expansion = false
  []
  [TKE]
    type = INSFVEnergyVariable
    solver_sys = TKE_system
    initial_condition = ${k_init}
    two_term_boundary_expansion = false
  []
  [TKED]
    type = INSFVEnergyVariable
    solver_sys = TKED_system
    initial_condition = ${eps_init}
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu_t'
    momentum_component = 'x'
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_t'
    momentum_component = 'y'
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []

  [p_diffusion]
    type = FVAnisotropicDiffusion
    variable = pressure
    coeff = "Ainv"
    coeff_interp_method = 'average'
  []
  [p_source]
    type = FVDivergence
    variable = pressure
    vector_field = "HbyA"
    force_boundary_execution = true
  []

  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
  []
  [temp_conduction]
    type = FVDiffusion
    coeff = ${k}
    variable = T_fluid
  []
  [temp_turb_conduction]
    type = FVDiffusion
    coeff = 'k_t'
    variable = T_fluid
  []

  [TKE_advection]
    type = INSFVTurbulentAdvection
    variable = TKE
    rho = ${rho}
  []
  [TKE_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = ${mu}
  []
  [TKE_diffusion_turbulent]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = 'mu_t'
    scaling_coef = ${sigma_k}
  []
  [TKE_source_sink]
    type = INSFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    epsilon = TKED
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    walls = ${walls}
    wall_treatment = ${wall_treatment_v}
  []

  [TKED_advection]
    type = INSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    walls = ${walls}
  []
  [TKED_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = ${mu}
    walls = ${walls}
  []
  [TKED_diffusion_turbulent]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = 'mu_t'
    scaling_coef = ${sigma_eps}
    walls = ${walls}
  []
  [TKED_source_sink]
    type = INSFVTKEDSourceSink
    variable = TKED
    u = vel_x
    v = vel_y
    k = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    walls = ${walls}
    wall_treatment = ${wall_treatment_v}
  []
[]

[FVBCs]
  [top_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top'
    function = ${lid_velocity}
  []
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'left right bottom'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'left right top bottom'
    function = 0
  []
  [T_hot]
    type = INSFVTurbulentTemperatureWallFunction
    variable = T_fluid
    boundary = 'top'
    T_w = 1
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    cp = ${cp}
    kappa = ${k}
    k = TKE
    wall_treatment = ${wall_treatment_T}
  []
  [T_cold]
    type = INSFVTurbulentTemperatureWallFunction
    variable = T_fluid
    boundary = 'bottom'
    T_w = 0
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    cp = ${cp}
    kappa = ${k}
    k = TKE
    wall_treatment = ${wall_treatment_T}
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'left right top bottom'
    variable = mu_t
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    k = TKE
    wall_treatment = ${wall_treatment_v}
  []
[]

[AuxVariables]
  [mu_t]
    type = MooseVariableFVReal
    initial_condition = '${fparse rho * C_mu * ${k_init}^2 / eps_init}'
    two_term_boundary_expansion = false
  []
  [k_t]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
[]

[AuxKernels]
  [compute_mu_t]
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    k = TKE
    epsilon = TKED
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    bulk_wall_treatment = ${bulk_wall_treatment}
    walls = ${walls}
    wall_treatment = ${wall_treatment_v}
    execute_on = 'NONLINEAR'
  []
  [compute_k_t]
    type = TurbulentConductivityAux
    variable = k_t
    Pr_t = ${Pr_t}
    cp = ${cp}
    mu_t = 'mu_t'
    execute_on = 'NONLINEAR'
  []
[]

[FunctorMaterials]
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T_fluid'
    rho = ${rho}
    cp = ${cp}
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  turbulence_systems = 'TKED_system TKE_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.5
  energy_equation_relaxation = 0.9
  turbulence_equation_relaxation = '0.8 0.8'
  num_iterations = 500
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  energy_absolute_tolerance = 1e-12
  turbulence_absolute_tolerance = '1e-12 1e-12'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  momentum_l_abs_tol = 1e-14
  energy_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  turbulence_l_abs_tol = 1e-14
  momentum_l_max_its = 30
  pressure_l_max_its = 30
  momentum_l_tol = 0.0
  energy_l_tol = 0.0
  pressure_l_tol = 0.0
  turbulence_l_tol = 0.0
  print_fields = false
  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.01 0.099 0.0'
  continue_on_max_its = true
[]

[Outputs]
  csv = true
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]

[VectorPostprocessors]
  [side_bottom]
    type = SideValueSampler
    boundary = 'bottom'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [side_top]
    type = SideValueSampler
    boundary = 'top'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [side_left]
    type = SideValueSampler
    boundary = 'left'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'y'
    execute_on = 'timestep_end'
  []
  [side_right]
    type = SideValueSampler
    boundary = 'right'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'y'
    execute_on = 'timestep_end'
  []
  [horizontal_center]
    type = LineValueSampler
    start_point = '${fparse 0.01 * side_length} ${fparse 0.499 * side_length} 0'
    end_point = '${fparse 0.99 * side_length} ${fparse 0.499 * side_length} 0'
    num_points = ${Mesh/gen/nx}
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [vertical_center]
    type = LineValueSampler
    start_point = '${fparse 0.499 * side_length} ${fparse 0.01 * side_length} 0'
    end_point = '${fparse 0.499 * side_length} ${fparse 0.99 * side_length} 0'
    num_points =  ${Mesh/gen/ny}
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'y'
    execute_on = 'timestep_end'
  []
[]
