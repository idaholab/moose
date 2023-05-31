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
    ix = '50'
    iy = '10  3'
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
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [mu_t]
    type = MooseVariableFVReal
    initial_condition = '10.0'
  []
  [ax_out]
    type = MooseVariableFVReal
  []
  [ay_out]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
  [ax_out]
    type = ADFunctorElementalAux
    functor = ax
    variable = ax_out
    execute_on = timestep_end
  []
  [ay_out]
    type = ADFunctorElementalAux
    functor = ay
    variable = ay_out
    execute_on = timestep_end
  []
[]

[Problem]
  previous_nl_solution_required = true
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
[]

[Debug]
  show_var_residual_norms = true
[]

[Executioner]
  type = Steady
  end_time = 4
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
  petsc_options_value = 'lu        NONZERO               0.7'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-2
  nl_max_its = 2000
  line_search = none

  # Fixed point iteration parameters
  fixed_point_max_its = 30
  accept_on_max_fixed_point_iteration = true
  fixed_point_abs_tol = 1e-8
  relaxation_factor = 1.0
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [turb]
    type = FullSolveMultiApp
    input_files = 'channel_simple_test_turb.i'
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [u_to_turb]
    type = MultiAppCopyTransfer
    to_multi_app = 'turb'
    source_variable = 'vel_x'
    variable = 'vel_x'
  []
  [v_to_turb]
    type = MultiAppCopyTransfer
    to_multi_app = 'turb'
    source_variable = 'vel_y'
    variable = 'vel_y'
  []
  [p_to_turb]
    type = MultiAppCopyTransfer
    to_multi_app = 'turb'
    source_variable = 'pressure'
    variable = 'pressure'
  []
  [ax]
    type = MultiAppCopyTransfer
    to_multi_app = 'turb'
    source_variable = ax_out
    variable = ax
    execute_on = 'timestep_end'
  []
  [ay]
    type = MultiAppCopyTransfer
    to_multi_app = 'turb'
    source_variable = ay_out
    variable = ay
    execute_on = 'timestep_end'
  []
  [mu_t_to_main]
    type = MultiAppCopyTransfer
    from_multi_app = 'turb'
    source_variable = 'mu_t'
    variable = 'mu_t'
  []
  [u_from_turb]
    type = MultiAppCopyTransfer
    from_multi_app = 'turb'
    source_variable = 'vel_x'
    variable = 'vel_x'
  []
  [v_from_turb]
    type = MultiAppCopyTransfer
    from_multi_app = 'turb'
    source_variable = 'vel_y'
    variable = 'vel_y'
  []
  [p_from_turb]
    type = MultiAppCopyTransfer
    from_multi_app = 'turb'
    source_variable = 'pressure'
    variable = 'pressure'
  []
[]
