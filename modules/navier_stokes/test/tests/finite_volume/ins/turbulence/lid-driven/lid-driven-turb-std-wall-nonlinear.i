##########################################################
# Lid-driven cavity test
# Reynolds: 5,000
# Author: Dr. Mauricio Tano
# Last Update: November, 2023
# Turbulent model using:
# k-epsilon model
# No wall functions
# Newton Solve
##########################################################

### Thermophysical Properties ###
mu = 2e-5
rho = 1.0

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
walls = ''
linearized_model = false

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
    nx = 10
    ny = 10
  []
[]

[Problem]
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-10
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-10
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 0.2
  []
  [TKE]
    type = INSFVEnergyVariable
    initial_condition = ${k_init}
  []
  [TKED]
    type = INSFVEnergyVariable
    initial_condition = ${eps_init}
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
  []
  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
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
    mu_interp_method = average
  []
  [u_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu_t'
    momentum_component = 'x'
    complete_expansion = true
    u = vel_x
    v = vel_y
    mu_interp_method = average
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
    mu_interp_method = average
  []
  [v_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_t'
    momentum_component = 'y'
    complete_expansion = true
    u = vel_x
    v = vel_y
    mu_interp_method = average
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  [TKE_time]
    type = FVFunctorTimeKernel
    variable = TKE
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
    coeff_interp_method = average
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
    linearized_model = ${linearized_model}
  []

  [TKED_time]
    type = FVFunctorTimeKernel
    variable = TKED
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
    coeff_interp_method = average
  []
  [TKED_source_sink]
    type = INSFVTKEDSourceSink
    variable = TKED
    u = vel_x
    v = vel_y
    tke = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    walls = ${walls}
    linearized_model = ${linearized_model}
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
  [walls_TKE]
    type = FVDirichletBC
    boundary = 'left right top bottom'
    variable = TKE
    value = ${k_init}
  []
  [walls_TKED]
    type = FVDirichletBC
    boundary = 'left right top bottom'
    variable = TKED
    value = ${eps_init}
  []
[]

[FunctorMaterials]
  [mu_t_material]
    type = INSFVkEpsilonViscosityFunctorMaterial
    tke = TKE
    epsilon = TKED
    rho = ${rho}
  []
[]

[Executioner]
  type = Transient
  end_time = 200
  dt = 0.01
  steady_state_detection = true
  steady_state_tolerance = 1e-3
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -snes_linesearch_damping'
  petsc_options_value = 'lu        NONZERO               0.5'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 50
  line_search = none
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = true
  print_linear_residuals = true
[]
