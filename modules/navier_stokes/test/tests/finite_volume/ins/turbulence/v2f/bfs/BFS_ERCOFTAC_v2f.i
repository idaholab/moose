##########################################################
# ERCOFTAC test case foe BFS
# Case Number: 031
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

### k-epslilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Initial and Boundary Conditions ###
intensity = 0.1
k_init = '${fparse 1.5*(intensity * bulk_u)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / H}'

### Modeling parameters ###
non_equilibrium_treatment = true
walls = 'bottom wall-side top'
max_mixing_length = 1e10

[Mesh]
  uniform_refine = 0
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${fparse 10.0*H} ${fparse 20.0*H}'
    dy = '${H} ${fparse 5*H}'
    ix = '8 16'
    iy = '6 20'
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
  nl_sys_names = 'u_system v_system pressure_system TKE_system TKED_system TV2_system TF_system'
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
    nl_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
    nl_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 1e-8
    nl_sys = pressure_system
    two_term_boundary_expansion = false
  []
  [TKE]
    type = INSFVEnergyVariable
    nl_sys = TKE_system
    initial_condition = ${k_init}
  []
  [TKED]
    type = INSFVEnergyVariable
    nl_sys = TKED_system
    initial_condition = ${eps_init}
  []
  [TV2]
    type = INSFVEnergyVariable
    nl_sys = TV2_system
    initial_condition = '${fparse 2/3*k_init}'
  []
  [TF]
    type = INSFVEnergyVariable
    nl_sys = TF_system
    initial_condition = 0.0
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
    mu = 'mu_t_v2f'
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
    mu = 'mu_t_v2f'
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
    coeff = 'mu_t_v2f'
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
    mu_t = 'mu_t_v2f'
    walls = ${walls}
    non_equilibrium_treatment = ${non_equilibrium_treatment}
    max_mixing_length = ${max_mixing_length}
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
    coeff = 'mu_t_v2f'
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
    mu_t = 'mu_t_v2f'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    walls = ${walls}
    non_equilibrium_treatment = ${non_equilibrium_treatment}
    max_mixing_length = ${max_mixing_length}
    v2f_formulation = true
    v2 = TV2
  []

  [TV2_advection]
    type = INSFVTurbulentAdvection
    variable = TV2
    rho = ${rho}
  []
  [TV2_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TV2
    coeff = ${mu}
  []
  [TV2_diffusion_turbulent]
    type = INSFVTurbulentDiffusion
    variable = TV2
    coeff = 'mu_t_v2f'
    scaling_coef = ${sigma_k}
  []
  [TV2_source_sink]
    type = INSFVTV2SourceSink
    variable = TV2
    u = vel_x
    v = vel_y
    k = TKE
    epsilon = TKED
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_v2f'
    f = TF
  []

  [TF_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TF
    coeff = 1.0
  []
  [TF_source_sink]
    type = INSFVTFSourceSink
    variable = TF
    u = vel_x
    v = vel_y
    k = TKE
    epsilon = TKED
    v2 = TV2
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t_v2f'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    functor = '${bulk_u}'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    functor = 0
  []
  [walls-u]
    type = FVDirichletBC
    boundary = 'bottom wall-side top'
    variable = vel_x
    value = 0
  []
  [walls-v]
    type = FVDirichletBC
    boundary = 'bottom wall-side top'
    variable = vel_y
    value = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    functor = 0
  []
  [inlet_TKE]
    type = INSFVInletIntensityTKEBC
    boundary = 'left'
    variable = TKE
    u = vel_x
    v = vel_y
    intensity = ${intensity}
  []
  [inlet_TV2]
    type = INSFVInletIntensityTKEBC
    boundary = 'left'
    variable = TV2
    u = vel_x
    v = vel_y
    intensity = '${fparse 2.0/3.0*intensity}'
  []
  [inlet_TKED]
    type = INSFVMixingLengthTKEDBC
    boundary = 'left'
    variable = TKED
    k = TKE
    characteristic_length = '${fparse H}'
  []
  [walls_func_TV2]
    type = INSFVTV2WallFunctionBC
    boundary = 'bottom wall-side top'
    variable = TV2
    rho = ${rho}
    mu = ${mu}
    k = TKE
  []
  [walls_func_TF]
    type = INSFVTFWallFunctionBC
    boundary = 'bottom wall-side top'
    variable = TF
    rho = ${rho}
    mu = ${mu}
    k = TKE
    epsilon = TKED
    v2 = TV2
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'bottom wall-side top'
    variable = mu_t_v2f
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    k = 'TKE'
    mu_t = 'mu_t_v2f'
  []
[]

[AuxVariables]
  [mu_t_v2f]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
    initial_condition = 10.0
  []
[]

[AuxKernels]
  [compute_mu_t_v2f]
    type = v2fViscosityAux
    variable = 'mu_t_v2f'
    k = TKE
    epsilon = TKED
    v2 = TV2
    mu = ${mu}
    rho = ${rho}
    execute_on = 'NONLINEAR'
  []
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  turbulence_systems = 'TKED_system TKE_system TV2_system TF_system'

  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  turbulence_equation_relaxation = '0.5 0.5 0.5 0.5'
  turbulence_iterations_to_activate = '10 10 2 2'
  turbulence_relaxation_decay_rate = '200 200 200 200'
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  turbulence_absolute_tolerance = '1e-12 1e-12 1e-12 1e-12'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  turbulence_petsc_options_iname = '-pc_type -pc_hypre_type'
  turbulence_petsc_options_value = 'hypre boomeramg'

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
  [console]
    type = Console
    outlier_variable_norms = false
  []
[]
