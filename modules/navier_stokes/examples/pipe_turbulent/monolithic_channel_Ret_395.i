von_karman_const = 0.41

H = 1 #halfwidth of the channel
L = 150

Re = 13700

rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * 2 * H / Re}'

advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${L}'
    dy = '0.667 0.333'
    ix = '200'
    iy = '10  4'
  []
[]

# Crafted wall function
f = '${fparse 0.316 * Re^(-0.25)}'
ref_delta_P = '${fparse f * L / (2*H) * rho * bulk_u^2 / 2}'
tau_wall = '${fparse ref_delta_P / (pi * (2*H) * L)}'
u_tau = '${fparse sqrt(tau_wall / rho)}'
y_dist_wall = '${fparse (2*H)/11/2}'
mu_wall = '${fparse rho * pow(u_tau,2) * y_dist_wall / bulk_u}'

# Crafted bulk viscosity
turbulent_intensity = 0.01 #'${fparse 0.16 * pow(Re, -1.0/8.0)}'
C_mu = 0.09
mixing_length = '${fparse (2*H) * 0.07}'
k_bulk = '${fparse 3/2 * pow(bulk_u*turbulent_intensity, 2)}'
eps_bulk = '${fparse pow(C_mu, 0.75) * pow(k_bulk, 1.5) / mixing_length}'
mu_bulk = '${fparse rho * C_mu * pow(k_bulk, 2) / eps_bulk}'

sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92

diff = 10.0

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  velocity_interp_method = ${velocity_interp_method}
  two_term_boundary_expansion = true
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
    initial_condition = ${bulk_u}
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [TKE]
    type = INSFVEnergyVariable
    initial_condition = 1e-10
    #initial_condition = ${k_bulk}
    #scaling = '${fparse 1/k_bulk}'
  []
  [TKED]
    type = INSFVEnergyVariable
    initial_condition = 1e-10
    #initial_condition = ${eps_bulk}
    #scaling = '${fparse 1/eps_bulk}'
  []
[]

[FVKernels]

  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
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
    complete_expansion = true
    u = vel_x
    v = vel_y
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
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  [TKE_advection]
    type = INSFVTurbulentAdvection
    variable = TKE
    rho = ${rho}
    walls = 'top'
  []
  [TKE_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = 'mu_t'
    scaling_coef = ${sigma_k}
    walls = 'top'
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
    linearized_model = true
    rf = 0.7
    walls = 'top'
    non_equilibrium_treatement = false
    iters_to_activate = 5
    relaxation_method = 'nl'
    top_production_bound = 5
    top_destruction_bound = 5
  []

  [TKED_advection]
    type = INSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    walls = 'top'
  []
  [TKED_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = 'mu_t'
    scaling_coef = ${sigma_eps}
    walls = 'top'
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
    rf = 0.7
    walls = 'top'
    # non_equilibrium_treatement = false
    iters_to_activate = 10
    relaxation_method = 'nl'
    top_production_bound = 5
    top_destruction_bound = 5
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [mu_t]
    type = MooseVariableFVReal
    initial_condition = '${fparse mu_bulk}'
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
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    k = TKE
    epsilon = TKED
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    wall_treatement = false
    walls = 'top'
    non_equilibrium_treatement = false
    rf = 1e-2
    execute_on = 'NONLINEAR'
    relaxation_method = 'nl'
    iters_to_activate = 20
    mu_t_inital = 0.1
  []
[]

[Problem]
  previous_nl_solution_required = true
[]

[Functions]
  # Not working
  [viscous_jump]
    type = ADParsedFunction
    expression = 'if((abs(y) > (D)*(11/2 -1)/(11/2)), mu_wall, mu_bulk)'
    symbol_names = 'D mu_wall mu_bulk'
    symbol_values = '${fparse 2*H} ${mu_wall} ${mu_bulk}'
  []
[]

[Materials]
  [viscosity]
    type = ADGenericFunctorMaterial
    prop_names = 'mu_t_imposed'
    prop_values = 'viscous_jump'
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
    intensity = ${turbulent_intensity}
  []
  [inlet_TKED]
    type = INSFVMixingLengthTKEDBC
    boundary = 'left'
    variable = TKED
    k = TKE
    characteristic_length = '${fparse 2*H}'
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'top'
    variable = mu_t
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = mu_t
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

[Debug]
  show_var_residual_norms = true
[]

[Executioner]
  type = Steady
  end_time = 100
  dt = 1
  # [TimeStepper]
  #   type = IterationAdaptiveDT
  #   dt = 0.001
  #   iteration_window = 2
  #   optimal_iterations = 10
  #   growth_factor = 1.2
  #   cutback_factor = 0.8
  # []
  steady_state_detection = true
  steady_state_tolerance = 1e-6
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -snes_linesearch_damping'
  petsc_options_value = 'lu        NONZERO               0.5'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 2000
  line_search = none
[]

[Outputs]
  exodus = true
[]
