Re = 1000000
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
I = 0.1
k_0 = '${fparse max(1.5 * (I^2 * bulk_u^2), 1e-6)}'
# eps_0 = '${fparse C_mu^0.75 * k_0^1.5 / l_0 / 2}'
visc_ratio = 100.0
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
    nx = 50
    ny = 10
    bias_y = '${fparse 1 / 1.2}'
  []
[]

[Variables]
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
    initial_condition = ${fparse C_mu * (k_0^2) / eps_0}
  []
  [y_plus]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1.0
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_viscosity_rans]
    type = INSFVMomentumDiffusionComplete
    variable = vel_x
    momentum_component = 'x'
    mu = 'mu_t'
    u = vel_x
    v = vel_y
    harmonic_interpolation = true
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
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_viscosity_rans]
    type = INSFVMomentumDiffusionComplete
    variable = vel_y
    momentum_component = 'y'
    mu = 'mu_t'
    u = vel_x
    v = vel_y
    harmonic_interpolation = true
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
    realizable_constraint = true
  []

  [TKED_advection]
    type = PINSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
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
    realizable_constraint = true
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
[]

[FVBCs]
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
  # [inlet-TKE]
  #   type = FVDirichletBC
  #   boundary = 'left'
  #   variable = TKE
  #   value = ${fparse 0.07*D*1^2}
  # []
  # [inlet-TKED]
  #   type = FVDirichletBC
  #   boundary = 'left'
  #   variable = TKED
  #   value = ${fparse 0.07*D*1^2}
  # []
  [inlet-TKE]
    type = NSFVTKEInletBC
    boundary = 'left'
    variable = TKE
    value = ${k_0} #${fparse 0.07*D*1^2}
    rho = ${rho}
    u = 'vel_x'
    v = 'vel_y'
  []
  [inlet-TKED]
    type = NSFVTKEInletBC
    boundary = 'left'
    variable = TKED
    value = ${eps_0} #${fparse 0.07*D*1^2}
    rho = ${rho}
    u = 'vel_x'
    v = 'vel_y'
  []
  [wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = vel_x
    function = 0
  []
  [wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = vel_y
    function = 0
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
#   [wall-TKED]
#     type = NSFVTKEDWallFunctionBC
#     boundary = 'top'
#     variable = TKED
#     k = TKE
#     rho = ${rho}
#     mu_t = ${mu} #'mu_t'
#     C_mu = ${C_mu}
#   []
#   [wall-TKED]
#     type = FVDirichletBC
#     boundary = 'top'
#     variable = TKED
#     value = ${eps_0} #'1'
#   []
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
  [sym-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = mu_t
    momentum_component = x
  []
  [sym-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = mu_t
    momentum_component = y
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
#   [out_TKE]
#     type = FVScalarOutflowBC
#     boundary = 'right'
#     variable = TKE
#     u = 'vel_x'
#     v = 'vel_y'
#   []
#   [out_TKED]
#     type = FVScalarOutflowBC
#     boundary = 'right'
#     variable = TKED
#     u = 'vel_x'
#     v = 'vel_y'
#   []
[]

# [Bounds]
#     [positive_TKE]
#       type = ConstantBoundsAux
#       variable = TKE
#       bounded_variable = TKE
#       bound_type = lower
#       bound_value = 0
#     []
#     [positive_TKED]
#         type = ConstantBoundsAux
#         variable = TKED
#         bounded_variable = TKED
#         bound_type = lower
#         bound_value = 0
#     []
# []

[Dampers]
    [limit_TKE]
      type = BoundingValueElementDamperFV
      variable = 'TKE'
      max_value = 1e10
      min_value = 0.0
      min_damping = 0.001
    []
    [limit_TKED]
        type = BoundingValueElementDamperFV
        variable = 'TKED'
        max_value = 1e10
        min_value = 0.0
        min_damping = 0.001
    []
[]


# [Preconditioning]
#   [SMP]
#     type = SMP
#     full = true
#   []
# []

# [Preconditioning]

#     [FSP]
#         type = FSP
#         # It is the starting point of splitting
#         topsplit = 'nuv' # 'uv' should match the following block name

#         [nuv]
#         splitting = 'a b c'
#         splitting_type = multiplicative
#         #petsc_options = '-dm_view'
#         []
#         [a]
#             vars = 'vel_x vel_y pressure'
#             petsc_options = '-ksp_monitor'
#             petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
#             petsc_options_value = 'lu NONZERO 50'
#         []
#         [b]
#             vars = 'TKE'
#             #petsc_options = '-ksp_monitor'
#             petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
#             petsc_options_value = 'lu NONZERO 50'
#             #full = true
#         []
#         [c]
#             vars = 'TKED'
#             #petsc_options = '-ksp_monitor'
#             petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
#             petsc_options_value = 'lu NONZERO 50'
#             #full = true
#         []
#     []
# []

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1.0
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'lu NONZERO 50'
#   petsc_options_iname = '-pc_type -pc_svd_monitor'
#   petsc_options_value = 'svd true'
#   line_search = none
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  automatic_scaling = true

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