Re = 4e4
D = 1
rho = 1000
# bulk_u = 1
# mu = '${fparse rho * bulk_u * D / Re}'

mu = 1e-3
bulk_u = '${fparse Re * mu / (D * rho)}'

sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

## Initialization
# nu_0 = 1.0
# l_0 = ${fparse 0.07*D}
I = 0.01
k_0 = '${fparse max(3.0/2.0 * (I^2 * bulk_u^2), 1e-6)}'
#eps_0 = '${fparse C_mu^0.75 * k_0^1.5 / l_0 / 2}'
visc_ratio = 10.0
eps_0 = '${fparse C_mu * rho * k_0^2 / (mu * visc_ratio)}'

advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    a_u = ax
    a_v = ay
    pressure = pressure
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = '${fparse 0.5 * D}'
    nx = 100
    ny = 20
    bias_y = '${fparse 1 / 1.2}'
  []
[]

[Variables]
  [TKE]
    type = INSFVEnergyVariable
    initial_condition = ${k_0}
  []
  [TKED]
    type = INSFVEnergyVariable
    initial_condition = ${eps_0}
  []
[]

[AuxVariables]
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
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = ${bulk_u}
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [linearized_epsilon_k]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = '${fparse k_0/eps_0}'
  []
  [ax]
    type = MooseVariableFVReal
  []
  [ay]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [TKE_advection]
    type = PINSFVTurbulentAdvection
    variable = TKE
    rho = ${rho}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
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
    linearized_model = true
    linear_variable = linearized_epsilon_k
  []

  [TKED_advection]
    type = PINSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []

  [TKED_diffusion]
    #type = PINSFVEnergyDiffusion
    type = PINSFVTurbulentDiffusion
    variable = TKED
    mu_t = 'mu_t'
    porosity = 1
    turb_coef = ${sigma_eps}
    #k = ${sigma_eps}
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
    linearized_model = true
    linear_variable = linearized_epsilon_k
  []
[]

[AuxKernels]
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
    walls = 'top'
    linearized_yplus = false
    n_iters_activate = 0
    execute_on = 'TIMESTEP_END'
    wall_treatement = true
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
  # [relax_mut]
  #   type = relaxVar
  #   variable = 'mu_t'
  #   var_to_relax = 'mu_t'
  #   relaxation_factor = 0.1
  #   execute_on = 'NONLINEAR'
  # []
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
    execute_on = 'TIMESTEP_END'
  []
[]

[FVBCs]
  [inlet-TKE]
    type = NSFVTKEInletBC
    boundary = 'left'
    variable = TKE
    rho = ${rho}
    u = 'vel_x'
    v = 'vel_y'
    turbulent_intensity = ${I}
  []
  [inlet-TKED]
    type = NSFVTKEDTurbulentRatioInletBC
    boundary = 'left'
    variable = TKED
    rho = ${rho}
    mu = ${mu}
    u = 'vel_x'
    v = 'vel_y'
    turbulent_intensity = ${I}
    C_mu = ${C_mu}
    turbulent_ratio = ${visc_ratio}
  []
  #   [wall-TKE]
  #     type = FVDirichletBC
  #     boundary = 'top'
  #     variable = TKE
  #     value = ${k_0} #'1'
  #   []
  [wall-TKE]
    type = NSFVTKEWallFunctionBC
    boundary = 'top'
    variable = TKE
  []
  # [wall-TKED]
  #   type = NSFVTKEDWallFunctionBC
  #   boundary = 'top'
  #   variable = TKED
  #   k = TKE
  #   rho = ${rho}
  #   mu_t = ${mu} #'mu_t'
  #   C_mu = ${C_mu}
  # []
  # [wall-TKED]
  #   type = FVDirichletBC
  #   boundary = 'top'
  #   variable = TKED
  #   value = ${eps_0} #'1'
  # []
  [wall-TKED]
    type = NSFVTKEDWallFunctionReichardtBC
    boundary = 'top'
    variable = TKED
    k = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C_mu = ${C_mu}
    u = vel_x
    v = vel_y
  []
  # [out_TKE]
  #   type = FVScalarOutflowBC
  #   boundary = 'right'
  #   variable = TKE
  #   u = 'vel_x'
  #   v = 'vel_y'
  # []
  # [out_TKED]
  #   type = FVScalarOutflowBC
  #   boundary = 'right'
  #   variable = TKED
  #   u = 'vel_x'
  #   v = 'vel_y'
  # []
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = ${bulk_u}
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0'
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

# [Dampers]
#   [limit_TKE]
#     type = BoundingValueElementDamperFV
#     variable = 'TKE'
#     max_value = 1e10
#     min_value = 0.1
#     min_damping = 0.1
#   []
#   [limit_TKED]
#       type = BoundingValueElementDamperFV
#       variable = 'TKED'
#       max_value = 1e10
#       min_value = 0.1
#       min_damping = 0.1
#   []
# []

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1.0
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -pc_factor_shift_type'
  petsc_options_value = 'hypre 20 NONZERO'
  nl_max_its = 20
  # petsc_options_iname = '-pc_type -pc_svd_monitor'
  # petsc_options_value = 'svd true'
  # petsc_options = '-ksp_view_pmat'
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-2
  automatic_scaling = false
[]

[Debug]
  show_var_residual_norms = true
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = false
[]

[Outputs]
  exodus = true
[]
