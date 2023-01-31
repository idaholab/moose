Re = 4e4
D = 1
rho = 1000
# bulk_u = 1
# mu = '${fparse rho * bulk_u * D / Re}'

mu = 1e-3
bulk_u = '${fparse Re * mu / (D * rho)}'

# sigma_k = 1.0
# sigma_eps = 1.3
# C1_eps = 1.44
# C2_eps = 1.92
C_mu = 0.09

## Initialization
# nu_0 = 1.0
# l_0 = ${fparse 0.07*D}
I = 0.01
k_0 = '${fparse max(1.5 * (I^2 * bulk_u^2), 1e-6)}'
# eps_0 = '${fparse C_mu^0.75 * k_0^1.5 / l_0 / 2}'
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
  [ax_out]
    type = MooseVariableFVReal
  []
  [ay_out]
    type = MooseVariableFVReal
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
[]

[AuxKernels]
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
[]

[AuxKernels]
  [relax_mut]
    type = relaxVar
    variable = 'mu_t'
    var_to_relax = 'mu_t'
    relaxation_factor = 1.0
    execute_on = 'NONLINEAR'
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
#             vars = 'vel_x'
#             petsc_options = '-ksp_monitor'
#             petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
#             petsc_options_value = 'lu NONZERO 50'
#         []
#         [b]
#             vars = 'vel_y'
#             #petsc_options = '-ksp_monitor'
#             petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
#             petsc_options_value = 'lu NONZERO 50'
#             #full = true
#         []
#         [c]
#             vars = 'pressure'
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
  petsc_options_iname = '-pc_type -ksp_gmres_restart -pc_factor_shift_type'
  petsc_options_value = 'lu 200 NONZERO'
  nl_max_its = 200
  #   petsc_options_iname = '-pc_type -pc_svd_monitor'
  #   petsc_options_value = 'svd true'
  #   line_search = none
  # petsc_options = '-ksp_view_pmat'
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-2
  automatic_scaling = true

[]

[Debug]
  show_var_residual_norms = true
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = false
[]

###### MultiApp for kEpsilon #####
[MultiApps]
  [kEpsilon]
    type = TransientMultiApp
    input_files = '2d-rc-kepsilon-subapp.i'
    execute_on = 'TIMESTEP_END'
  []
[]

[Transfers]
  [velocity_x]
    type = MultiAppCopyTransfer
    to_multi_app = kEpsilon
    source_variable = vel_x
    variable = vel_x
  []

  [velocity_y]
    type = MultiAppCopyTransfer
    to_multi_app = kEpsilon
    source_variable = vel_y
    variable = vel_y
  []

  [ax]
    type = MultiAppCopyTransfer
    source_variable = ax_out
    variable = ax
    to_multi_app = kEpsilon
  []
  [ay]
    type = MultiAppCopyTransfer
    source_variable = ay_out
    variable = ay
    to_multi_app = kEpsilon
  []

  [pressure]
    type = MultiAppCopyTransfer
    to_multi_app = kEpsilon
    source_variable = pressure
    variable = pressure
  []

  [turbulent_viscosity]
    type = MultiAppCopyTransfer
    from_multi_app = kEpsilon
    source_variable = mu_t
    variable = mu_t
  []
[]

[Outputs]
  exodus = true
[]
