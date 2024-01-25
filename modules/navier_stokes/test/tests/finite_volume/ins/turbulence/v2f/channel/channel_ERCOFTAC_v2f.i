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

H = 1 #halfwidth of the channel
L = 30

Re = 13700

rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * 2 * H / Re}'

advected_interp_method = 'upwind'

pressure_tag = "pressure_grad"

### k-epslilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

epsilon_scaling = 1.0

### Initial and Boundary Conditions ###
intensity = 0.1
k_init = '${fparse 1.5*(intensity * bulk_u)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / H / epsilon_scaling}'

### Modeling parameters ###
non_equilibrium_treatment = true
bulk_wall_treatment = false
walls = ''
max_mixing_length = 1e10
linearized_yplus_mu_t = false

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = 0
    ymax = ${H}
    nx = 20
    ny = 8
    bias_y = 0.7
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
    characteristic_length = '${fparse H/epsilon_scaling}'
  []
  [walls_TKE]
    type = FVDirichletBC
    boundary = 'top'
    variable = TKE
    value = 0.0
  []
  [walls_hom_TV2]
    type = FVDirichletBC
    boundary = 'top'
    variable = TV2
    value = 0.0
  []
  [walls_func_TV2]
    type = INSFVTV2WallFunctionBC
    boundary = 'top'
    variable = TV2
    rho = ${rho}
    mu = ${mu}
    k = TKE
  []
  [walls_hom_TF]
    type = FVDirichletBC
    boundary = 'top'
    variable = TF
    value = 0.0
  []
  [walls_func_TF]
    type = INSFVTFWallFunctionBC
    boundary = 'top'
    variable = TF
    rho = ${rho}
    mu = ${mu}
    k = TKE
    epsilon = TKED
    v2 = TV2
  []
  [walls_TKED]
    type = INSFVTKEDWallFunctionBC
    boundary = 'top'
    variable = TKED
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    k = TKE
  []
  [sym-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = 'mu_t'
    momentum_component = x
  []
  [sym-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = 'mu_t'
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
  [symmetry_TKED]
    type = INSFVSymmetryScalarBC
    boundary = 'bottom'
    variable = TKED
  []
[]

[AuxVariables]
  [mu_t]
    type = MooseVariableFVReal
    initial_condition = '${fparse rho * C_mu * ${k_init}^2 / eps_init}'
    two_term_boundary_expansion = false
  []
  [mu_t_v2f]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
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
    linearized_yplus = ${linearized_yplus_mu_t}
    non_equilibrium_treatment = ${non_equilibrium_treatment}
    execute_on = 'NONLINEAR'
  []
  [compute_mu_t_v2f]
    type = v2fViscosityAux
    variable = mu_t_v2f
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
  turbulence_systems = 'TKED_system TKE_system TF_system TV2_system'

  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  turbulence_equation_relaxation = '0.5 0.5 0.5 0.5'
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  turbulence_absolute_tolerance = '1e-10 1e-10 1e-10 1e-10'
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
