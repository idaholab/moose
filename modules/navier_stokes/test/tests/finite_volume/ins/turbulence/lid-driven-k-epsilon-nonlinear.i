mu = 0.00001
rho = 1
lid_velocity = 0.1

sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

## Initialization
k_0 = 1.0
eps_0 = 10.0

[GlobalParams]
  velocity_interp_method = 'rc'
  advected_interp_method = 'average'
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
  []
  [vel_y]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
  [TKE]
    type = INSFVEnergyVariable
    initial_condition = ${k_0}
    scaling = 1e4
  []
  [TKED]
    type = INSFVEnergyVariable
    initial_condition = ${eps_0}
    scaling = 1e4
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [mu_t]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = '${fparse rho * C_mu * (k_0^2) / eps_0}'
  []
  [y_plus]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1.0
  []
  [linearized_epsilon_k]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = '${fparse k_0/eps_0}'
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
  [compute_mu_t]
    type = kEpsilonViscosity
    variable = mu_t
    k = TKE
    epsilon = TKED
    rho = ${rho}
    C_mu = ${C_mu}
    mu = ${mu}
    u = vel_x
    v = vel_y
    walls = 'left right top bottom'
    linearized_yplus = false
    n_iters_activate = 0
    execute_on = 'NONLINEAR'
    wall_treatement = false
    non_equilibrium_treatement = false
  []
  [compute_y_plus]
    type = WallFunctionYPlusAux
    variable = y_plus
    rho = ${rho}
    mu = ${mu}
    u = vel_x
    v = vel_y
    walls = 'top'
  []
  [compute_linear_variable]
    type = kEpsilonLinearVariable
    variable = linearized_epsilon_k
    k = TKE
    epsilon = TKED
    u = vel_x
    v = vel_y
    C_mu = ${C_mu}
    rho = ${rho}
    walls = 'top'
    mu = ${mu}
    execute_on = 'NONLINEAR'
  []
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
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
    phi0 = 0.0
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
    mu = 'mu_t'
    momentum_component = 'x'
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
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_t'
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  [TKE_advection]
    type = PINSFVTurbulentAdvection
    variable = TKE
    rho = ${rho}
  []
  [TKE_diffusion]
    type = PINSFVTurbulentDiffusion
    variable = TKE
    mu_t = 'mu_t'
    porosity = 1
    turb_coef = ${sigma_k}
  []
  [TKE_SourceSink]
    type = PINSFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu_t = 'mu_t'
    effective_balance = false
    epsilon = TKED
    porosity = 1
    realizable_constraint = false
    linearized_model = false
    linear_variable = linearized_epsilon_k
  []

  [TKED_advection]
    type = PINSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
  []
  [TKED_diffusion]
    type = PINSFVTurbulentDiffusion
    variable = TKED
    mu_t = 'mu_t'
    porosity = 1
    turb_coef = ${sigma_eps}
  []
  [TKED_SourceSink]
    type = PINSFVTKEDSourceSink
    variable = TKED
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu_t = 'mu_t'
    effective_balance = false
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    k = TKE
    porosity = 1
    realizable_constraint = false
    linearized_model = false
    linear_variable = linearized_epsilon_k
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

  [wall-TKE]
    type = NSFVTKEWallFunctionBC
    boundary = 'left right top bottom'
    variable = TKE
  []

  [wall-TKED]
    type = NSFVTKEDWallFunctionReichardtBC
    boundary = 'left right top bottom'
    variable = TKED
    k = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C_mu = ${C_mu}
    u = vel_x
    v = vel_y
  []
[]

[Materials]
  [mu]
    type = ADGenericFunctorMaterial
    prop_names = 'mu'
    prop_values = '${mu}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-4
  nl_max_its = 100
  residual_and_jacobian_together = true
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
[]
