##########################################################
# ERCOFTAC test case foe turbulent channel flow
# Case Number: 032
# Author: Dr. Mauricio Tano
# Last Update: Novomber, 2023
# Turbulent model using:
# k-epsilon model
# Equilibrium + Newton wall treatement
# SIMPLE solve
##########################################################

Re = 5100

rho = 1.0
bulk_u = 1.0
H = 1.0
mu = '${fparse rho * bulk_u * H/ Re}'

advected_interp_method = 'upwind'

pressure_tag = "pressure_grad"

### Initial and Boundary Conditions ###
intensity = 0.01
k_init = '${fparse 1.5*(intensity * bulk_u)^2}'
omega_init = '${fparse k_init^0.5 / (2*H)}'

### Modeling parameters ###
bulk_wall_treatment = false
walls = 'bottom wall-side top'
wall_treatment = 'eq_newton' # Options: eq_newton, eq_incremental, eq_linearized, neq

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${fparse 10.0*H} ${fparse 20.0*H}'
    dy = '${H} ${fparse 5*H}'
    ix = '8 16'
    iy = '2 8'
    subdomain_id = '
                    2 1
                    1 1
                  '
  []
  [corner_walls]
    type = SideSetsBetweenSubdomainsGenerator
    input = gen
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'wall-side'
  []
  [delete_bottom]
    type = BlockDeletionGenerator
    input = corner_walls
    block = '2'
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system TKE_system TKESD_system'
  previous_nl_solution_required = true
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  velocity_interp_method = 'rc'
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
    initial_condition = ${bulk_u}
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 1e-8
    solver_sys = pressure_system
    two_term_boundary_expansion = false
  []
  [TKE]
    type = INSFVEnergyVariable
    solver_sys = TKE_system
    initial_condition = ${k_init}
  []
  [TKESD]
    type = INSFVEnergyVariable
    solver_sys = TKESD_system
    initial_condition = ${omega_init}
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
    mu = 'mu_t_k_omega'
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
    mu = 'mu_t_k_omega'
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
    coeff = 'mu_t_k_omega'
    scaling_coef = 'sigma_k'
  []
  [TKE_source_sink]
    type = INSFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    # epsilon = TKESD
    omega = TKESD
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_k_omega'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    F1 = F1
  []

  [TKESD_advection]
    type = INSFVTurbulentAdvection
    variable = TKESD
    rho = ${rho}
    walls = ${walls}
  []
  [TKESD_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKESD
    coeff = ${mu}
    walls = ${walls}
  []
  [TKESD_diffusion_turbulent]
    type = INSFVTurbulentDiffusion
    variable = TKESD
    coeff = 'mu_t_k_omega'
    scaling_coef = 'sigma_omega'
    walls = ${walls}
  []
  # [TKESD_source_sink]
  #   type = INSFVTKESDSourceSink
  #   variable = TKESD
  #   u = vel_x
  #   v = vel_y
  #   k = TKE
  #   rho = ${rho}
  #   mu = ${mu}
  #   mu_t = 'mu_t'
  #   C1_eps = ${C1_eps}
  #   C2_eps = ${C2_eps}
  #   walls = ${walls}
  #   wall_treatment = ${wall_treatment}
  # []
  [TKESD_source_sink]
    type = INSFVTKESDSourceSink
    variable = TKESD
    u = vel_x
    v = vel_y
    k = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_k_omega'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    F1 = F1
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '${bulk_u}'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = 0
  []
  [walls-u]
    type = FVDirichletBC
    boundary = ${walls}
    variable = vel_x
    value = 0
  []
  [walls-v]
    type = FVDirichletBC
    boundary = ${walls}
    variable = vel_y
    value = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0
  []
  [inlet_TKE]
    type = INSFVInletIntensityTKEBC
    boundary = 'left'
    variable = TKE
    u = vel_x
    v = vel_y
    intensity = ${intensity}
  []
  # [inlet_TKESD]
  #   type = INSFVMixingLengthTKEDBC
  #   boundary = 'left'
  #   variable = TKESD
  #   k = TKE
  #   characteristic_length = '${fparse 2*H}'
  # []
  [inlet_TKESD]
    type = FVDirichletBC
    boundary = 'left'
    variable = TKESD
    value = ${omega_init}
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = ${walls}
    variable = 'mu_t_k_omega'
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_k_omega'
    k = TKE
    wall_treatment = ${wall_treatment}
  []
[]

[AuxVariables]
  # [mu_t]
  #   type = MooseVariableFVReal
  #   initial_condition = '${fparse rho * C_mu * ${k_init}^2 / omega_init}'
  #   two_term_boundary_expansion = false
  # []
  # [yplus]
  #   type = MooseVariableFVReal
  #   two_term_boundary_expansion = false
  # []
  [wall_distance]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [F1]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [F2]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [sigma_k]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [sigma_omega]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
  [mu_t_k_omega]
    type = MooseVariableFVReal
    initial_condition = '${fparse rho * k_init / omega_init}'
    two_term_boundary_expansion = false
  []
[]

[AuxKernels]
  # [compute_mu_t]
  #   type = kEpsilonViscosityAux
  #   variable = mu_t
  #   C_mu = ${C_mu}
  #   k = TKE
  #   epsilon = TKESD
  #   mu = ${mu}
  #   rho = ${rho}
  #   u = vel_x
  #   v = vel_y
  #   bulk_wall_treatment = ${bulk_wall_treatment}
  #   walls = ${walls}
  #   wall_treatment = ${wall_treatment}
  #   execute_on = 'NONLINEAR'
  # []
  # [compute_y_plus]
  #   type = RANSYPlusAux
  #   variable = yplus
  #   k = TKE
  #   mu = ${mu}
  #   rho = ${rho}
  #   u = vel_x
  #   v = vel_y
  #   walls = ${walls}
  #   wall_treatment = ${wall_treatment}
  #   execute_on = 'NONLINEAR'
  # []
  [compute_wall_distance]
    type = WallDistanceAux
    variable = wall_distance
    walls = ${walls}
    execute_on = 'INITIAL NONLINEAR'
  []
  [compute_F1]
    type = kOmegaSSTF1BlendingAux
    variable = F1
    k = TKE
    omega = TKESD
    rho = ${rho}
    mu = ${mu}
    wall_distance = wall_distance
    execute_on = 'NONLINEAR'
  []
  [compute_F2]
    type = kOmegaSSTF2BlendingAux
    variable = F2
    k = TKE
    omega = TKESD
    rho = ${rho}
    mu = ${mu}
    wall_distance = wall_distance
    execute_on = 'NONLINEAR'
  []
  [compute_sigma_k]
    type = kOmegaSSTSigmaKAux
    variable = sigma_k
    F1 = F1
    execute_on = 'NONLINEAR'
  []
  [compute_sigma_omega]
    type = kOmegaSSTSigmaKAux
    variable = sigma_omega
    F1 = F1
    execute_on = 'NONLINEAR'
  []
  [compute_mu_t_k_omega]
    type = kOmegaSSTViscosityAux
    variable = 'mu_t_k_omega'
    k = TKE
    omega = TKESD
    mu = ${mu}
    rho = ${rho}
    F2 = F2
    u = vel_x
    v = vel_y
    bulk_wall_treatment = ${bulk_wall_treatment}
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  turbulence_systems = 'TKESD_system TKE_system'

  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.5
  turbulence_equation_relaxation = '0.8 0.8'
  num_iterations = 3000
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  turbulence_absolute_tolerance = '1e-12 1e-12'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  turbulence_l_abs_tol = 1e-14
  momentum_l_max_its = 30
  pressure_l_max_its = 30
  momentum_l_tol = 0.0
  pressure_l_tol = 0.0
  turbulence_l_tol = 0.0
  print_fields = false
[]

[Outputs]
  exodus = true
[]
