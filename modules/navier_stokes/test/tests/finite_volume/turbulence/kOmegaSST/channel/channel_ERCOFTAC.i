##########################################################
# ERCOFTAC test case foe turbulent channel flow
# Case Number: 032
# Author: Dr. Mauricio Tano
# Last Update: Novomber, 2023
# Turbulent model using:
# k-omega-SST model
# Equilibrium + Newton wall treatement
# SIMPLE solve
##########################################################

H = 1 #halfwidth of the channel
L = 120

Re = 22250

rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * 2 * H / Re}'

advected_interp_method = 'upwind'

pressure_tag = "pressure_grad"

### Initial and Boundary Conditions ###
intensity = 0.01
k_init = '${fparse 1.5*(intensity * bulk_u)^2}'
omega_init = 0.166

### Modeling parameters ###
bulk_wall_treatment = false
walls = 'top'
wall_treatment = 'neq' # Options: eq_newton, eq_incremental, eq_linearized, neq

### Model corrections
low_Re_modification = false
free_shear_modification = false
vortex_stretching_modficiation = false

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = 0
    ymax = ${H}
    nx = 20
    ny = 5
    bias_y = 0.7
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
    omega = TKESD
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_k_omega'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    F1 = F1
    free_shear_modification = ${free_shear_modification}
    low_Re_modification = ${low_Re_modification}
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
    free_shear_modification = ${free_shear_modification}
    wall_normal_unit_vectors = 'wall_cell_face_normals'
    vortex_stretching_modficiation = ${vortex_stretching_modficiation}
    low_Re_modification = ${low_Re_modification}
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
    boundary = 'top'
    variable = vel_x
    value = 0
  []
  [walls-v]
    type = FVDirichletBC
    boundary = 'top'
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
  [inlet_TKESD]
    type = INSFVMixingLengthTKESDBC
    boundary = 'left'
    variable = TKESD
    k = TKE
    characteristic_length = '${fparse 2*H}'
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'top'
    variable = 'mu_t_k_omega'
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_k_omega'
    k = TKE
    wall_treatment = ${wall_treatment}
  []
  [sym-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = 'mu_t_k_omega'
    momentum_component = x
  []
  [sym-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = 'mu_t_k_omega'
    momentum_component = y
  []
  [symmetry_pressure]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
  [symmetry_TKE]
    type = INSFVSymmetryScalarBC
    boundary = 'bottom'
    variable = TKE
  []
  [symmetry_TKESD]
    type = INSFVSymmetryScalarBC
    boundary = 'bottom'
    variable = TKESD
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
[]

[AuxKernels]
  [compute_wall_distance]
    type = WallDistanceAux
    variable = wall_distance
    walls = 'top'
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
    walls = 'top'
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
  pressure_variable_relaxation = 0.3
  turbulence_equation_relaxation = '0.25 0.25'
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
