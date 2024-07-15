##########################################################
# Lid-driven cavity test
# Reynolds: 5,000
# Author: Dr. Mauricio Tano
# Last Update: November, 2023
# Turbulent model using:
# k-omega-SST model
# Standard wall functions
# SIMPLE Solve
##########################################################

### Thermophysical Properties ###
Re = 5000
rho = 1.0
lid_velocity = 1.0
side_length = 0.1
mu = ${fparse rho*lid_velocity*side_length/Re}

### Initial Conditions
intensity = 0.01
k_init = '${fparse 1.5*(intensity * lid_velocity)^2}'
omega_init = '${fparse k_init^0.5 / side_length}'

### Modeling parameters
bulk_wall_treatment = false
walls = ''
wall_treatment = 'neq' # Options: eq_newton, eq_incremental, eq_linearized, neq

### Model corrections
low_Re_modification = false
free_shear_modification = true
vortex_stretching_modficiation = false

# bias
bias = 1.3
L = ${side_length}
N = 6

pressure_tag = "pressure_grad"

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'rc'
[]

[Mesh]
  [top_left]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${fparse L / 2.0}
    ymin = ${fparse L / 2.0}
    ymax = ${fparse L}
    nx = ${N}
    ny = ${N}
    bias_x = ${fparse bias}
    bias_y = ${fparse 1.0 / bias}
  []

  [top_right]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = ${fparse L / 2.0}
    xmax = ${fparse L}
    ymin = ${fparse L / 2.0}
    ymax = ${fparse L}
    nx = ${N}
    ny = ${N}
    bias_x = ${fparse 1.0 / bias}
    bias_y = ${fparse 1.0 / bias}
  []

  [bottom_left]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${fparse L / 2.0}
    ymin = 0
    ymax = ${fparse L / 2.0}
    nx = ${N}
    ny = ${N}
    bias_x = ${fparse bias}
    bias_y = ${fparse bias}
  []

  [bottom_right]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = ${fparse L / 2.0}
    xmax = ${fparse L}
    ymin = 0.0
    ymax = ${fparse L / 2.0}
    nx = ${N}
    ny = ${N}
    bias_x = ${fparse 1.0 / bias}
    bias_y = ${fparse bias}
  []

  [tops]
    type = StitchedMeshGenerator
    inputs = 'top_left top_right'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'right left'
    prevent_boundary_ids_overlap = 'false'
  []
  [bottoms]
    type = StitchedMeshGenerator
    inputs = 'bottom_left bottom_right'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'right left'
    prevent_boundary_ids_overlap = 'false'
  []
  [all]
    type = StitchedMeshGenerator
    inputs = 'tops bottoms'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'bottom top'
    prevent_boundary_ids_overlap = 'false'
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system TKE_system TKESD_system'
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
    mu_interp_method = 'average'
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
    mu_interp_method = 'average'
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
    coeff_interp_method = 'average'
  []
  [TKE_source_sink]
    type = INSFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    omega = 'TKESD'
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_k_omega'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
  []

  [TKESD_advection]
    type = INSFVTurbulentAdvection
    variable = 'TKESD'
    rho = ${rho}
    walls = ${walls}
  []
  [TKESD_diffusion]
    type = INSFVTurbulentDiffusion
    variable = 'TKESD'
    coeff = ${mu}
    walls = ${walls}
  []
  [TKESD_diffusion_turbulent]
    type = INSFVTurbulentDiffusion
    variable = 'TKESD'
    coeff = 'mu_t_k_omega'
    scaling_coef = 'sigma_omega'
    walls = ${walls}
  []
  [TKESD_source_sink]
    type = INSFVTKESDSourceSink
    variable = 'TKESD'
    u = vel_x
    v = vel_y
    k = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_k_omega'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    F1 = F1
    free_shear_modification = ${free_shear_modification}
    wall_normal_unit_vectors = 'wall_cell_face_normals'
    vortex_stretching_modficiation = ${vortex_stretching_modficiation}
    low_Re_modification = ${low_Re_modification}
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
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'left right top bottom'
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
  [wall_cell_face_normals]
    type = VectorMooseVariable
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [y_plus]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
[]

[AuxKernels]
  [compute_wall_distance]
    type = WallDistanceAux
    variable = wall_distance
    walls = 'left top bottom right'
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
    low_Re_modification = ${low_Re_modification}
    F1 = F1
    execute_on = 'NONLINEAR'
  []
  [compute_cell_wall_normals]
    type = WallNormalUnitVectorAux
    variable = 'wall_cell_face_normals'
    walls = 'left top bottom right'
  []
  [compute_y_plus]
    type = RANSYPlusAux
    variable = 'y_plus'
    walls = 'left top bottom right'
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    k = TKE
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  turbulence_systems = 'TKESD_system TKE_system'

  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.5
  turbulence_equation_relaxation = '0.7 0.7'
  num_iterations = 500
  pressure_absolute_tolerance = 1e-${N}
  momentum_absolute_tolerance = 1e-${N}
  turbulence_absolute_tolerance = '1e-${N} 1e-${N}'
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

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.01 0.099 0.0'
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
