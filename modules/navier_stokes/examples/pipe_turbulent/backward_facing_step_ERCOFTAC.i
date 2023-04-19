von_karman_const = 0.41

Re = 5100

rho = 1.0
bulk_u = 1.0
h = 1.0
mu = '${fparse rho * bulk_u * h/ Re}'

advected_interp_method = 'upwind'
velocity_interp_method = 'average'

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${fparse 10.0*h} ${fparse 20.0*h}'
    dy = '${h} ${fparse 5*h}'
    ix = '40 80'
    iy = '8  32'
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
  # [fmg]
  #   type = FileMeshGenerator
  #   file = backward_facing_step_in.e
  # []
[]

# Crafted wall function
f = '${fparse 0.316 * Re^(-0.25)}'
ref_delta_P = '${fparse f * (30*h) / (5*h) * rho * bulk_u^2 / 2}'
tau_wall = '${fparse ref_delta_P / (pi * (5*h) * (30*h))}'
u_tau = '${fparse sqrt(tau_wall / rho)}'
y_dist_wall = '${fparse (5*h)/11/2}'
mu_wall = '${fparse rho * pow(u_tau,2) * y_dist_wall / bulk_u}'

# Crafted bulk viscosity
turbulent_intensity = 0.05 #'${fparse 0.16 * pow(Re, -1.0/8.0)}'
C_mu = 0.09
mixing_length = '${fparse (5*h) * 0.07}'
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
    walls = 'bottom wall-side'
  []
  [TKE_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = 'mu_t'
    scaling_coef = ${sigma_k}
    walls = 'bottom wall-side'
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
    rf = 1.0
    walls = 'bottom wall-side'
    non_equilibrium_treatement = false
  []

  [TKED_advection]
    type = INSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    walls = 'bottom wall-side'
  []
  [TKED_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = 'mu_t'
    scaling_coef = ${sigma_eps}
    walls = 'bottom wall-side'
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
    rf = 1.0
    walls = 'bottom wall-side'
    # non_equilibrium_treatement = false
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
    walls = 'bottom wall-side'
    non_equilibrium_treatement = false
    rf = 0.8
    execute_on = 'TIMESTEP_END'
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
    symbol_values = '${fparse 5*h} ${mu_wall} ${mu_bulk}'
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
    boundary = 'bottom wall-side'
    variable = vel_x
    value = 0
  []
  [walls-u-top]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top'
    variable = vel_x
    momentum_component = 'x'
  []
  [walls-v]
    type = FVDirichletBC
    boundary = 'top bottom wall-side'
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
    characteristic_length = '${fparse 5*h}'
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'bottom wall-side'
    variable = mu_t
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = mu_t
    k = TKE
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Executioner]
  type = Transient
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
  petsc_options_value = 'lu        NONZERO               0.9'
  nl_abs_tol = 1e-8
  nl_max_its = 2000
  line_search = none
[]

[Outputs]
  exodus = true
[]
